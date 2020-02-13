// Copyright � 2015-2020 Alex Kukhtin. All rights reserved.

#pragma once

//class CFPCheckItemInfo;
//class CFiscalPrinterInfo;

class CFPException
{
	UINT m_nID;
	std::wstring _error;
public:
	CFPException(unsigned nID)
		: m_nID(nID) {};
	CFPException(const wchar_t* szError)
		: m_nID(0), _error(szError) {};
	//void ReportError2();
	const wchar_t* GetError();
};

struct RECEIPT_ITEM {
	__int64 article;
	const wchar_t* name;
	const wchar_t* unit;
	__currency vat; /* percent * 100 */
	int qty;
	__currency weight;
	__currency price;
	__currency sum;
	__currency discount;
	RECEIPT_ITEM()
		: article(0), qty(0), name(nullptr), unit(nullptr) {}
};

struct PAYMENT_INFO {
	__currency sumCash;
	__currency sumGet;
	__currency sumCard;

	PAYMENT_INFO() {}
};

enum PAYMENT_MODE {
	_pay_cash,
	_pay_card
};

class FiscalPrinterImpl
{
public:
	std::wstring _id;
	std::wstring m_strError;

	static ITraceTarget* _traceTarget;

	enum PrinterFlags
	{
		FP_SYNCTIME = 0x0001,
		FP_MODEMSTATE = 0x0002
	};


	FiscalPrinterImpl(void);
	virtual ~FiscalPrinterImpl();

	const wchar_t* GetLastError();

	virtual bool IsOpen() const;
	virtual bool IsReady() const;
	virtual bool Open(const wchar_t* Port, DWORD nBaudRate) = 0;
	virtual void Init();
	virtual void Close();
	virtual DWORD GetFlags();

	virtual int GetLastReceiptNo(__int64 termId, bool bFromPrinter = false) = 0;
	virtual LONG GetCurrentZReportNo(__int64 termId, bool bFromPrinter = false) = 0;
	//virtual bool FillZReportInfo(ZREPORT_INFO& zri);

	//virtual void SetPrinterInfo(const CFiscalPrinterInfo& info);
	//virtual bool PrintDiagnostic();
	virtual void XReport()  = 0;
	virtual void ZReport() = 0;
	virtual void NullReceipt(bool bOpenCashDrawer) = 0;
	//virtual bool PostNullCheck(__int64 hid);
	//virtual bool PostClose(__int64 hid);
	//virtual bool ProgramOperator(const wchar_t* Name, const wchar_t* Password);
	virtual bool CancelReceipt(__int64 termId, bool& bClosed) = 0;
	virtual bool CancelReceiptCommand(__int64 termId) = 0;
	virtual void OpenReceipt() = 0;
	virtual void OpenReturnReceipt() = 0;
	virtual void PrintReceiptItem(const RECEIPT_ITEM& item) = 0;
	virtual void Payment(PAYMENT_MODE mode, long sum) = 0;
	virtual void CloseReceipt() = 0;
	//virtual bool PrintCheckItem(const CFPCheckItemInfo& info) = 0;
	virtual bool PrintDiscount(long Type, long Sum, const wchar_t* szDescr);
	virtual bool PrintDiscountForAllReceipt(long dscPercent, long dscSum);
	//virtual bool CloseCheck(int sum, int get, CFiscalPrinter::PAY_MODE pm, const wchar_t* szText = nullptr);
	//virtual bool CloseCheck2(int sum, int ret, int get, CFiscalPrinter::PAY_MODE pm);
	virtual void ServiceInOut(__currency sum) = 0;
	//virtual bool PeriodicalByDate(BOOL Short, COleDateTime From, COleDateTime To);
	virtual bool PeriodicalByNo(BOOL Short, LONG From, LONG To) = 0;
	virtual bool CopyBill() = 0;
	virtual bool ReportByArticles() = 0;
	virtual bool ReportRems();
	virtual bool ReportModemState() = 0;
	virtual void AddArticle(const RECEIPT_ITEM& item) = 0;
	virtual void OpenCashDrawer() = 0;
	virtual void PrintFiscalText(const wchar_t* szText) = 0;
	virtual void PrintNonFiscalText(const wchar_t* szText);
	virtual void Beep();
	//virtual bool GetCash(__int64 termId, COleCurrency& cy);
	virtual void SetCurrentTime() = 0;
	virtual void DisplayDateTime() = 0;
	virtual void DisplayClear() = 0;
	virtual void DisplayRow(int rowNo, const wchar_t* szString) = 0;
	virtual bool IsEndOfTape();
	virtual const std::wstring& GetError() const;

	void TraceINFO(const wchar_t* info, ...);
	void TraceERROR(const wchar_t* info, ...);
	bool IsDebugMode() const;

	static void PosSetTraceTarget(ITraceTarget* target);
private:
	void Trace(ITraceTarget::TraceType type, const wchar_t* msg, va_list args);
};