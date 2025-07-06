#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <mount/Mount.hpp>

#include <lx200/lx200.h>

LOG_MODULE_REGISTER(main, CONFIG_FIRMWARE_LOG_LEVEL);

// Mount
Mount mount;

int main()
{

	// ReSharper disable CppDFAEndlessLoop
	while (true)
	// ReSharper restore CppDFAEndlessLoop
	{
		k_sleep(K_MSEC(100));
	}

	return 0;
}
