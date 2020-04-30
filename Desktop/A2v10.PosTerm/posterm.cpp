
#include "pch.h"
#include "framework.h"
#include "posterm.h"
#include "command.h"
#include "fiscalprinter.h"
#include "equipmentbase.h"
#include "fiscalprinterimpl.h"
#include "acqterminal.h"
#include "stringtools.h"

#pragma comment(lib,"../Lib/A2v10.StaticBase.lib")


void PosSetTraceTarget(ITraceTarget* target)
{
	EquipmentBaseImpl::SetTraceTarget(target);
}

pos_result_t PosConnectToPrinter(const wchar_t* model, const wchar_t* port, int baud)
{
	return FiscalPrinter::Connect(model, port, baud);
}

pos_result_t PosConnectToAcquiringTerminal(const wchar_t* model, const wchar_t* port, const wchar_t* log)
{
	return AcqTerminal::Connect(model, port, log);
}

pos_result_t PosProcessCommandA(const char* json, std::string& result)
{
	std::wstring wjson = A2W(json);
	std::wstring wresult;
	auto res = PosProcessCommand(wjson.c_str(), wresult);
	result = W2A(wresult.c_str());
	return res;
}


pos_result_t PosProcessCommand(const wchar_t* json, std::wstring& result)
{
	JsonParser parser;
	PosCommand cmd;
	try 
	{
		parser.SetTarget(&cmd);
		parser.Parse(json);
		result = L"{\"msgid\":";
		result.append(std::to_wstring(cmd._msgid));
		pos_result_t res;
		if (cmd._command == L"connect")
			res = cmd.ExecuteConnectCommand(result);
		else 
			res = cmd.ExecuteCommand(result);
		if (res == pos_result_t::_success)
			result.append(L", \"status\":\"success\"");
		result.append(L"}");
		return res;
	}
	catch (JsonException ex) {
		result.assign(ex.GetMessage());
		return pos_result_t::_invalid_json;
	}
	catch (EQUIPException ex) {
		result.assign(ex.GetError());
		return pos_result_t::_device_error;
	}
	catch (...) {
		result.assign(L"Unknown exception");
		return pos_result_t::_generic_error;
	}
	return pos_result_t::_success;
}

void PosShutDown()
{
	FiscalPrinter::ShutDown();
	AcqTerminal::ShutDown();
}

const wchar_t* PosErrorMessage(pos_result_t res)
{
	switch (res)
	{
	case _success:
		return nullptr;
	case _generic_error:
		return L"generic error";
	case _invalid_json:
		return L"invalid json";
	case _invalid_model:
		return L"invalid model";
	case _could_not_connect:
		return L"could not connect";
	case _already_connected:
		return L"already connected";
	case _device_not_found:
		return L"device not found";
	}
	return L"unknown error";
}
