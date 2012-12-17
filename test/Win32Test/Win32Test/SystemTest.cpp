
#include "unit_test/tut/tut/tut.hpp"
#include "Win32/System/system_helper.hpp"


namespace tut
{
    struct System
    {};

    typedef test_group<System> sys;
    typedef sys::object object;
    sys system_group("win32::system");

    template<>
    template<>
    void object::test<1>()
    {
        set_test_name("Time SystemTime");
        SYSTEMTIME systemTime = {0};
        GetLocalTime(&systemTime);
        systemTime.wMilliseconds = 0;
        time_t time = ::time(0);
        ensure_equals("SystemTimeToTime", win32::system::system_time_to_time(systemTime), time);
        ensure_equals("TimeToSystemTime", memcmp(&win32::system::time_to_system_time(time), &systemTime, sizeof(systemTime)), 0);
    }

    template<>
    template<>
    void object::test<2>()
    {
        set_test_name("start process");

		utility::handle_ptr process = ::win32::system::start_process("c:\\windows\\notepad.exe");
        ensure(process);

		::WaitForSingleObject(process, INFINITE);

        Sleep(1);
    }

    template<>
    template<>
    void object::test<3>()
    {
        set_test_name("kill process");

        ensure(::win32::system::kill_process("notepad.exe"));
    }

    template<>
    template<>
    void object::test<4>()
    {
        set_test_name("get token from process name");

        ensure(::win32::system::get_token_from_process_name(_T("")).is_valid());
    }
}