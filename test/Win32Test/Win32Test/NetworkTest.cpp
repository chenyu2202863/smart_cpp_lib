#include "stdafx.h"

#include "unit_test/tut/tut/tut.hpp"
#include "../../../include/extend_stl/Unicode.hpp"
#include "win32/network/network_helper.hpp"

namespace tut
{
    struct Network{};

    typedef tut::test_group<Network> tg;
    typedef tg::object object;

    tg network_group("win32::network");

    template<>
    template<>
    void object::test<1>()
    {
        set_test_name("Test IP Availability");

        ensure("ip valid", ::win32::network::is_valid_ip("192.168.1.1"));
    }

    template<>
    template<>
    void object::test<2>()
    {
        set_test_name("Get Local IPs");

        std::vector<std::string> ipv;
        ensure("local ips", ::win32::network::get_local_ips(ipv));
    }

	template<>
	template<>
	void object::test<3>()
	{
		set_test_name("Get Local gate IPs");

		std::vector<std::string> ipv;
		ensure("gate ips", ::win32::network::get_local_gate_ips(ipv));
	}

	template<>
	template<>
	void object::test<4>()
	{
		set_test_name("Get Local dns IPs");

		std::vector<std::string> ipv;
		ensure("dns ips", win32::network::get_local_dns(ipv));
	}

	template<>
	template<>
	void object::test<5>()
	{
		set_test_name("MAC");

		std::string mac = ::win32::network::get_mac_addr("127.0.0.1");
		std::string val = win32::network::mac_string_to_binary(mac);
	}

	template<>
	template<>
	void object::test<6>()
	{
		set_test_name("local ip");

		ensure("local machine", win32::network::is_local_by_ip("127.0.0.1"));
		ensure("remote machine", !win32::network::is_local_by_ip("10.34.43.43"));
	}

	template<>
	template<>
	void object::test<7>()
	{
		set_test_name("is_port_use");

		ensure("5050", win32::network::is_port_used(5050));
		ensure("21", win32::network::is_port_used(21));
	}
}