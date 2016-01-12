#include "seh_eception.hpp"

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include <Windows.h>
#include <eh.h>
#include <Psapi.h>
#include <dbghelp.h>


#pragma comment(lib, "Dbghelp.lib")


namespace exception {

	namespace
	{
		HANDLE hProcess = 0;
		bool syms_ready = false;

		void init()
		{
			if( hProcess == 0 )
			{
				hProcess = ::GetCurrentProcess();
				::SymSetOptions(SYMOPT_DEFERRED_LOADS);

				if( ::SymInitialize(hProcess, NULL, TRUE) )
				{
					syms_ready = true;
				}
			}
		}
	}

	class seh_information_t
	{
	public:
		typedef unsigned int exception_code_t;

		static const char* op_desc(const ULONG opcode)
		{
			switch( opcode )
			{
			case 0: return "read";
			case 1: return "write";
			case 8: return "user-mode data execution prevention (DEP) violation";
			default: return "unknown";
			}
		}

		static const char* seDescription(const exception_code_t& code)
		{
			switch( code )
			{
			case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION";
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
			case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT";
			case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT";
			case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND";
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
			case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT";
			case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION";
			case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW";
			case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION_FLT_STACK_CHECK";
			case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW";
			case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION";
			case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR";
			case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO";
			case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW";
			case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION";
			case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
			case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION";
			case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP";
			case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW";
			default: return "UNKNOWN EXCEPTION";
			}
		}

		static std::string get_symbol(void *ptr)
		{
			if( ptr == 0 )
				return std::string();

			init();

			std::ostringstream ss;

			if( syms_ready )
			{
				DWORD64  dwDisplacement = 0;
				DWORD64  dwAddress = (DWORD64) ptr;

				std::vector<char> buffer(sizeof(SYMBOL_INFO) + MAX_SYM_NAME);
				PSYMBOL_INFO pSymbol = (PSYMBOL_INFO) &buffer.front();

				DWORD displaceMent = 0;
				IMAGEHLP_LINE line = { 0 };

				BOOL line_suc = SymGetLineFromAddr(hProcess, (DWORD) dwAddress, &displaceMent, &line);

				pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
				pSymbol->MaxNameLen = MAX_SYM_NAME;

				BOOL name_suc = ::SymFromAddr(hProcess, dwAddress, &dwDisplacement, pSymbol);
				if( line_suc && name_suc )
				{
					ss << line.FileName << "(" << line.LineNumber << "): ["
						<< pSymbol->Name << "]";
				}
				else
				{
					ss << "???";
				}
			}

			return ss.str();
		}

		static std::string information(struct _EXCEPTION_POINTERS* ep, bool has_exception_code = false, exception_code_t code = 0)
		{
			HMODULE hm;
			::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<LPCTSTR>(ep->ExceptionRecord->ExceptionAddress), &hm);
			MODULEINFO mi;
			::GetModuleInformation(::GetCurrentProcess(), hm, &mi, sizeof(mi));
			char fn[MAX_PATH] = {0};
			::GetModuleFileNameExA(::GetCurrentProcess(), hm, fn, MAX_PATH);

			std::ostringstream oss;
			oss << "SE " << (has_exception_code ? seDescription(code) : "") << " at address 0x" << std::hex << ep->ExceptionRecord->ExceptionAddress << std::dec
				<< " inside " << fn << " loaded at base address 0x" << std::hex << mi.lpBaseOfDll << "\n"
				<< get_symbol(ep->ExceptionRecord->ExceptionAddress) << std::endl;

			if( has_exception_code && (
				code == EXCEPTION_ACCESS_VIOLATION ||
				code == EXCEPTION_IN_PAGE_ERROR) )
			{
				oss << "Invalid operation: " << op_desc(ep->ExceptionRecord->ExceptionInformation[0]) << " at address 0x" << std::hex << ep->ExceptionRecord->ExceptionInformation[1] << std::dec << "\n";
			}

			if( has_exception_code && code == EXCEPTION_IN_PAGE_ERROR )
			{
				oss << "Underlying NTSTATUS code that resulted in the exception " << ep->ExceptionRecord->ExceptionInformation[2] << "\n";
			}

			return oss.str();
		}
	};


	void install_seh()
	{
		::_set_se_translator([](std::uint32_t code, EXCEPTION_POINTERS *ep)
		{
			throw std::exception(seh_information_t::information(ep, true, code).c_str());
		});
	}
}