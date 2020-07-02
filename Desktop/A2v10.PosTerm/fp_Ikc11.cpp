

#include "stdafx.h"
#include "..\include\fiscalprinter.h"
#include "fiscalprinterImpl.h"
#include "fp_IkcBase.h"
#include "fp_Ikc11.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFiscalPrinter_Ikc11::CFiscalPrinter_Ikc11()
{
}

// virtual 
bool CFiscalPrinter_Ikc11::Init(DB_ID termId)
{
	// %%%%
	return true;
}

// virtual
void CFiscalPrinter_Ikc11::CreateCommand(FP_COMMAND cmd, BYTE* pData /*= NULL*/, int DataLen /*= 0*/)
{
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
	ReadAll(); // ���, ��� ���� ������
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
			// ��������� ��������
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
				continue; // ������� DLE ����������
			m_rcvBuffer[rcvBytes++] = buff;
		}
		else
			::Sleep(20);
	}
	
	// ����� ���� ��� ������ ������
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
				continue; // ������� DLE ����������
			m_rcvBuffer[rcvBytes++] = buff;

		}
		else
			::Sleep(20);
	}

	if ((rcvBytes == 0) || ((rcvBytes > 0) && (m_rcvBuffer[0] == NAK)))
	{
		// �������� �������� ������� � ������ SEQUENCE
		if (nCount > RETRY_COUNT)
			ThrowInternalError(L"Invalid retry count");
		IncSeq(); // ����� ��� ���� ������ �������
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


	BYTE rcvSeq = m_rcvBuffer[3]; // ������������������
	BYTE rcvCmd = m_rcvBuffer[4]; // �������

	m_RcvDataLen = rcvBytes - 11;
	IncSeq();
	if (rcvBytes < 6)
	{
		CString msg;
		msg.Format(L"Invalid receive count (%d)", rcvBytes);
		ThrowInternalError(msg);
	}
	ParseStatus();
}


// virtual
void CFiscalPrinter_Ikc11::RecalcCrcSum()
{
	m_sndBuffer[2] = m_nSeq; // number
	m_sndBuffer[4 + m_LastDataLen] = CalcCheckSum(m_sndBuffer, m_LastDataLen);
}
#pragma pack(push, 1)

struct DAYREPORT_INFO
{
	WORD schecks;
	/* 5 * �������� ������ �� ��������� ������� � ������ �����*/
	BYTE sale0[16];
	BYTE sale1[16];
	BYTE sale2[16];
	BYTE sale3[16];

	BYTE x1[4];  // ������� ������� �� ��������
	BYTE x2[4];  // ������� ������ �� ��������
	BYTE cashin[4];  // ������� ����� ���������� �����

	WORD rchecks; //  ������� ����� ���������
	/* 5 * �������� ��������� �� ��������� ������� � ������ �����*/
	BYTE rsale0[16];
	BYTE rsale1[16];
	BYTE rsale2[16];
	BYTE rsale3[16];

	BYTE y1[4]; // ������� ������� �� ��������
	BYTE y2[4]; // ������� ������ �� ��������
	BYTE cashout[4]; // ������� ����� ��������� ������

	LONG GetCashIn()
	{
			return cashin[3] * 16777216 + cashin[2] * 65536 + cashin[1] * 256 + cashin[0];
	}
	LONG GetCashOut()
	{
			return cashout[3] * 16777216 + cashout[2] * 65536 + cashout[1] * 256 + cashout[0];
	}
};

#pragma pack(pop)


void CFiscalPrinter_Ikc11::DayReport_(void* Info)
{
	DAYREPORT_INFO* pInfo = reinterpret_cast<DAYREPORT_INFO*>(Info);
	CreateCommand(FP_GETDAYINFO);
	SendCommand();
	GetData((BYTE*)pInfo, sizeof(DAYREPORT_INFO));
}

// virtual 
int CFiscalPrinter_Ikc11::GetLastCheckNo(DB_ID termId, bool bFromPrinter /*= false*/)
{
	if (bFromPrinter)
	{
		DAYREPORT_INFO info = {0};
		try {
			DayReport_(&info);
		} 
		catch (CFPException ex)
		{
			ex.ReportError2();
			return false;
		}
		WORD ch = info.schecks; // bin
		if (m_bReturnCheck)
			ch = info.rchecks;
		m_nLastCheckNo = ch;
	}
	return m_nLastCheckNo;
}

// virtual
CString CFiscalPrinter_Ikc11::FPGetLastError()
{
	// s=0, err=0
	if ((m_dwStatus == 0) && (m_dwError == 0))
		return EMPTYSTR; // OK
	if (m_dwStatus != 0)
	{
		if (m_dwError == 0)
		{
			if (m_dwStatus & 0x1) // bit 0
			{
				if (m_dwReserved & 0x4) // bit2
					return L"� �������� ����������� ������";
				return L"������� �� �����";
			}
			else if (m_dwStatus & 0x2) // bit1
				return L"���������� ����������������� �������� ������ � ����";
			else if (m_dwStatus & 0x4) // bit2
				return L"������ ��� ������������ ���������� ������";
			else if (m_dwStatus & 0x8) // bit3
				return L"������������ ���� ��� ������ �����";
			else if (m_dwStatus & 0x10) // bit4
				return L"������ ����������";
			else if (m_dwStatus & 0x20) // bit5
				return L"���������� ����������������� �����";
			else if (m_dwStatus & 0x40) // bit6
				return L"�������� �������� ���������� �������";
			else if (m_dwStatus & 0x80) // bit7
				return L"������� �� ���������� ��� ��������� � ������ ������";
		}
	}
	switch (m_dwError) 
	{
		case 1: return L"������ ��������";
		case 2: return L"����������� ������";
		case 4: return L"���� ���������� ������";
		case 6: return L"�������� ���������� �������";
		case 8: return L"���������� ������ �����������"; 
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
		case 42: return L"������� ���������, �� ���� �����";
		case 43: return L"����� � ���� ������ ���������";
		case 44: return L"������� ���������, ��� ������";
		case 45: return L"������/������� ���������, �� ���� ������";
		case 46: return L"������� ��������� ����� ������ �����";
		case 47: return L"������������ ����������� �����";
		case 48: return L"������������ ����� ������ ����";
		case 50: return L"������� ���������, ���� �� ������";
	}
	return L"����� ������ ��������";
}

// virtual 
bool CFiscalPrinter_Ikc11::FillZReportInfo(ZREPORT_INFO& zri)
{
	try 
	{
		DAYREPORT_INFO info = {0};
		DayReport_(&info);
		int s = info.GetCashIn();
		zri.m_cash_in = CCyT::MakeCurrency(s / 100, s % 100);
		s = info.GetCashOut();
		zri.m_cash_out = CCyT::MakeCurrency(s / 100, s % 100);
	} 
	catch (CFPException ex)
	{
		ex.ReportError2();
		return false;
	}
	return true;
}
