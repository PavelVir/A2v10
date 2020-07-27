// Copyright � 2015-2020 Alex Kukhtin. All rights reserved.

#include "pch.h"
#include "posterm.h"
#include "equipmentbase.h"
#include "fiscalprinter.h"
#include "fiscalprinterimpl.h"
#include "fp_IkcBase.h"
#include "fp_Ikc11.h"
#include "stringtools.h"
#include "errors.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFiscalPrinter_Ikc11::CFiscalPrinter_Ikc11(const wchar_t* model)
	: CFiscalPrinter_IkcBase(model)
{
}

// virtual
void CFiscalPrinter_Ikc11::CreateCommand(const wchar_t* name, FP_COMMAND cmd, BYTE* pData /*= NULL*/, int DataLen /*= 0*/)
{
	TraceINFO(L"%s\r\n  SND:0x%02X %s", name, (int)cmd, _Byte2String(pData, DataLen).c_str());

	ClearBuffers();
	m_sndBuffer[0] = DLE;
	m_sndBuffer[1] = STX;
	m_sndBuffer[2] = m_nSeq; // number
	m_sndBuffer[3] = cmd; 
	for (int i=0; i<DataLen; i++)
		m_sndBuffer[4 + i] = pData[i];
	m_sndBuffer[4 + DataLen] = 0; // // checksum
	m_sndBuffer[5 + DataLen] = DLE;
	m_sndBuffer[6 + DataLen] = ETX;
	
	m_sndBuffer[4 + DataLen] = CalcCheckSum(m_sndBuffer, DataLen);

	m_bytesToSend = 7 + DataLen;
	m_LastDataLen = DataLen;
}

// virtual
void CFiscalPrinter_Ikc11::SendCommand()
{
	ReadAll(); // everything received before
	int nCount = 0;
start:
	int rcvBytes = 0;

	DWORD dwSent = 0;
	for (int i=0; i<m_bytesToSend; i++)
	{
		BYTE b = m_sndBuffer[i];
		DWORD dwByte = 0;
		if (!WriteFile(m_hCom, &b, 1, &dwByte, NULL)) 
		{
			CloseComPort();
			m_dwOsError = ::GetLastError();
			ThrowOsError(m_dwOsError);
		}
		if ((b == DLE) && (i >= 2) && (i < (m_bytesToSend - 4)))
		{
			// repeat sending
			if (!WriteFile(m_hCom, &b, 1, &dwByte, NULL)) {
				CloseComPort();
				m_dwOsError = ::GetLastError();
				ThrowOsError(m_dwOsError);
			}
		}
		dwSent++;
	}
	/*
	if (!WriteFile(m_hCom, m_sndBuffer, m_bytesToSend, &dwSent, NULL)) {
		CloseComPort();
		m_dwOsError = ::GetLastError();
		ThrowCommonError();
	}
	*/
	if (m_bytesToSend != dwSent) {
		ThrowInternalError(L"Invalid send bytes");
	}

read:
	BYTE buff = 0x0;
	DWORD dwRead = 0;
	int cnt = 0;
	bool bStx = false;
	while (ReadFile(m_hCom, &buff, 1, &dwRead, NULL) && (dwRead == 1)) {
		if (true) // (buff != SYN)
		{
			if (buff == STX)
				bStx = true;
			if ((!bStx) && (buff == SYN))
				continue;
			if ((!bStx) && (buff == ENQ))
				continue;
			if ((rcvBytes > 0) && (buff == DLE) && (m_rcvBuffer[rcvBytes-1] == DLE))
				continue; // skip double DLE
			m_rcvBuffer[rcvBytes++] = buff;
		}
		else
			::Sleep(20);
	}
	
	// there can be TWO datasets
	while (ReadFile(m_hCom, &buff, 1, &dwRead, NULL) && (dwRead == 1)) 
	{
		if (true) // (buff != SYN)
		{
			if (buff == STX)
				bStx = true;
			if ((!bStx) && (buff == SYN))
				continue;
			if ((!bStx) && (buff == ENQ))
				continue;
			if ((rcvBytes > 0) && (buff == DLE) && (m_rcvBuffer[rcvBytes-1] == DLE))
				continue; // skip double DLE
			m_rcvBuffer[rcvBytes++] = buff;

		}
		else
			::Sleep(20);
	}

	if ((rcvBytes == 0) || ((rcvBytes > 0) && (m_rcvBuffer[0] == NAK)))
	{
		// repeat sending the command with next SEQUENCE
		if (nCount > RETRY_COUNT)
			ThrowInternalError(L"Invalid retry count");
		IncSeq();
		RecalcCrcSum();
		nCount++;
		goto start;
	}
	
	if ((rcvBytes == 1) && (m_rcvBuffer[0] == ACK))
	{
		goto read;
	} 
	else if ((rcvBytes == 1) && (m_rcvBuffer[0] == SYN))
	{
		rcvBytes = 0;
		goto read;
	}


	BYTE rcvSeq = m_rcvBuffer[3]; // sequence
	BYTE rcvCmd = m_rcvBuffer[4]; // command

	m_RcvDataLen = rcvBytes - 11;
	// CRC - 2 bytes
	TraceINFO(L"  RCV:%s", _Byte2String(m_rcvBuffer + 8, m_RcvDataLen - 2).c_str());
	IncSeq();
	if (rcvBytes < 6)
	{
		//CString msg;
		//msg.Format(L"Invalid receive count (%d)", rcvBytes);
		//ThrowInternalError(msg);
	}
	ParseStatus();
}


// virtual
void CFiscalPrinter_Ikc11::RecalcCrcSum()
{
	m_sndBuffer[2] = m_nSeq; // number
	m_sndBuffer[4 + m_LastDataLen] = CalcCheckSum(m_sndBuffer, m_LastDataLen);
}


// virtual
std::wstring CFiscalPrinter_Ikc11::FPGetLastError()
{
	// TODO: errors
	// s=0, err=0
	if ((m_dwStatus == 0) && (m_dwError == 0))
		return L""; // OK
	if (m_dwStatus != 0)
	{
		if (m_dwError == 0)
		{
			if (m_dwStatus & 0x1) // bit 0
			{
				if (m_dwReserved & 0x4) // bit2
					return FP_E_RECEIPT_TAPE_OVER;
				return L"������� �� �����";
			}
			else if (m_dwStatus & 0x2) // bit1
				return FP_E_MODEM_ERROR;
			else if (m_dwStatus & 0x4) // bit2
				return L"������ ��� ������������ ���������� ������";
			else if (m_dwStatus & 0x8) // bit3
				return L"������������ ���� ��� ������ �����";
			else if (m_dwStatus & 0x10) // bit4
				return L"������ ����������";
			else if (m_dwStatus & 0x20) // bit5
				return FP_E_SHIFTEXPIRED;
			else if (m_dwStatus & 0x40) // bit6
				return L"�������� �������� ���������� �������";
			else if (m_dwStatus & 0x80) // bit7
				return FP_E_INVALID_COMMAND;
		}
	}
	switch (m_dwError) 
	{
		case 1: return FP_E_GENERIC_ERROR;
		case 2: return FP_E_RECEIPT_TAPE_OVER;
		case 4: return L"��� ��������� ���'��";
		case 6: return L"C������� ���������� �������";
		case 8: return L"Գ������� ���'��� �����������"; 
		case 10: return L"�� ���� ��������������";
		case 16: return L"������� ��������� � ������ ������";
		case 19: return L"������ ���������������� ��������";
		case 20: return L"������������ ����� ������";
		case 21: return L"������������ ������";
		case 22: return L"�������������� ����� (������, ������)";
		case 23: return L"��������� ������ �� ���������� ��� �� �����������, ������ �� ���������";
		case 24: return L"��� ����� �� ����������";
		case 25: return L"������������ ���� ��������";
		case 26: return L"���������� ���������� �������";
		case 27: return L"������������� ������� ������ ����� ���������� ������ ����";
		case 28: return L"������ � �������� ��������";
		case 30: return L"������ ������� ����/�������";
		case 31: return L"���������� ����������� � ����";
		case 32: return L"���������� ����������� ����������� ���������";
		case 33: return L"������������ �������� �������� �������";
		case 34: return L"������������ �������� �����";
		case 35: return L"����� ������� ������, ��� � �������� �����";
		case 36: return L"���� ������ ���� ���������� z-������";
		case 37: return L"������ ��� ������, ������� ���������";
		case 38: return L"������ ��� ������, ������� ���������";
		case 39: return L"������� ���������, ��� �� ������";
		case 41: return L"������� ��������� �� Z-������";
		case 42: return FP_E_NORECEIPTS;
		case 43: return L"����� � ���� ������ ���������";
		case 44: return L"������� ���������, ��� ������";
		case 45: return L"������/������� ���������, �� ���� ������";
		case 46: return L"������� ��������� ����� ������ �����";
		case 47: return L"������������ ����������� �����";
		case 48: return L"������������ ����� ������ ����";
		case 50: return L"������� ���������, ���� �� ������";
	}
	return FP_E_GENERIC_ERROR;
}
