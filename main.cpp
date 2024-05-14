#include <iostream>
#include "bsl_tool.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace std;

void print_usage();
void print_version();
int flash(po::variables_map &vm, po::parsed_options &parsed);               // flash subcommand
int reset(po::variables_map &vm, po::parsed_options &parsed);               // reset subcommand
int enter_bsl(po::variables_map &vm, po::parsed_options &parsed);           // enter_bsl subcommand
int read_binary_version(po::variables_map &vm, po::parsed_options &parsed); // read_binary_version subcommand

int main(int argc, char** argv) {
    try {
        // main options/commands
        po::options_description main_desc("Main options");
        main_desc.add_options()
            ("help,h", "produce help message")
            ("version,v", "print version")
            ("command", po::value<string>(), "command (flash, reset, enter_bsl, read_binary_version)")
            ("cmd-args", po::value<std::vector<std::string> >(), "arguments for command")
        ;

        po::positional_options_description p;
        p.add("command", 1);
        p.add("cmd-args", -1);

        // parse
        po::variables_map vm;
        po::parsed_options parsed = po::command_line_parser(argc, argv).
            options(main_desc).
            positional(p).
            allow_unregistered().
            run();
        po::store(parsed, vm);
        po::notify(vm);

        // print version
        if(vm.count("version")) {
            print_version();
            return 0;
        }

        // handle subcommands
        if(vm.count("command")) {
            string cmd = vm["command"].as<string>();
            if(cmd == "flash"){
                return flash(vm, parsed);
            } else if(cmd == "reset"){
                return reset(vm, parsed);
            } else if(cmd == "enter_bsl") {
                return enter_bsl(vm, parsed);
            } else if(cmd == "read_binary_version") {
                return read_binary_version(vm, parsed);
            } else {
                printf("Unknown command '%s'!\n\n", cmd.c_str());
                cout << main_desc << "\n";
                return 0;
            }
        }

        // handle help/no command
        if (vm.count("help") || !vm.count("command")) {
            print_usage();
            cout << main_desc << "\n";
            return 0;
        }
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }

    return 0;
}

int flash(po::variables_map &vm, po::parsed_options &parsed)
{
    try {
        // flash command options
        po::options_description desc("flash options");
        desc.add_options()
            ("help,h", "produce help message")
            ("serial-port,p", po::value<string>(), "serial port (e.g. /dev/ttyACM0)")
            ("firmware-file,i", po::value<string>(), "firmware file")
            ("enter-bsl", po::value<bool>()->default_value(true), "enter BSL mode via GPIOs (default: true)")
            ("verbose", po::value<int>()->default_value(0), "verbosity level 0-3 (default: 0)")
            ("force", po::value<bool>()->default_value(false), "Force the update (default: false)")
        ;

        po::positional_options_description p;
        p.add("serial-port", 1);
        p.add("firmware-file", 2);

        // erase command name
        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());

        // reparse
        po::store(po::command_line_parser(opts).options(desc).positional(p).run(), vm);

        if (vm.count("help") || !vm.count("serial-port") || !vm.count("firmware-file")) {
            cout << desc << "\n";
            printf("Usage: MSPM0_bsl_flasher flash <serial> <binary> [options]\n");
            printf("=> Example: MSPM0_bsl_flasher flash /dev/ttyACM0 /home/foo/bar.bin\n\n");
            return 0;
        }

        bool status;
        int verbose_level = vm["verbose"].as<int>();
        bool enter_bsl_gpio = vm["enter-bsl"].as<bool>();
        const char* serial_path = vm["serial-port"].as<string>().c_str();
        const char* file_path = vm["firmware-file"].as<string>().c_str();
        uint32_t size = 0;

        auto b = BSLTool(serial_path, enter_bsl_gpio, verbose_level);
        b.open_file(file_path, size);
        std::string fw_version = b.read_file_version();
        printf("Using serial %s to flash %s\nFirmware version:%s\n\n", serial_path, file_path, fw_version.c_str());

        if(enter_bsl_gpio) {
            printf("Entering BSL mode\n");
            status = b.enter_bsl();
            if(!status) {
                printf("Could not enter BSL mode. Stopping...\n");
                return status;
            }
        }

        status = b.flash_image(file_path, vm["force"].as<bool>());
        return !status;
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }

    return 0;
}

int reset(po::variables_map &vm, po::parsed_options &parsed)
{
    try {
        // reset command options
        po::options_description desc("reset options");
        desc.add_options()
            ("help,h", "produce help message")
            ("verbose", po::value<int>()->default_value(0), "verbosity level 0-3 (default: 0)")
        ;

        // erase command name
        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());

        // reparse
        po::store(po::command_line_parser(opts).options(desc).run(), vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            printf("Usage: MSPM0_bsl_flasher reset [options]\n");
            printf("=> Example: MSPM0_bsl_flasher reset\n\n");
            return 0;
        }

        bool status;
        int verbose_level = vm["verbose"].as<int>();

        auto gpio = BSL_GPIO(verbose_level);
        printf("Resetting via GPIO\n");
        status = gpio.hard_reset();

        return status;
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }

    return 0;
}

int enter_bsl(po::variables_map &vm, po::parsed_options &parsed)
{
    try {
        // enter_bsl command options
        po::options_description desc("enter_bsl options");
        desc.add_options()
            ("help,h", "produce help message")
            ("verbose", po::value<int>()->default_value(0), "verbosity level 0-3 (default: 0)")
        ;

        // erase command name
        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());

        // reparse
        po::store(po::command_line_parser(opts).options(desc).run(), vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            printf("Usage: MSPM0_bsl_flasher enter_bsl [options]\n");
            printf("=> Example: MSPM0_bsl_flasher enter_bsl\n\n");
            return 0;
        }

        bool status;
        int verbose_level = vm["verbose"].as<int>();

        auto gpio = BSL_GPIO(verbose_level);
        printf("Entering BSL mode\n");
        status = gpio.enter_bsl();

        return status;
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }

    return 0;
}

int read_binary_version(po::variables_map &vm, po::parsed_options &parsed)
{
    try {
        // read_binary_version command options
        po::options_description read_version_desc("read_binary_version options");
        read_version_desc.add_options()
            ("help,h", "produce help message")
            ("firmware-file,i", po::value<string>(), "firmware file")
            ("verbose", po::value<int>()->default_value(0), "verbosity level 0-3 (default: 0)")
        ;

        po::positional_options_description p;
        p.add("firmware-file", 1);

        // erase command name
        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());

        // reparse
        po::store(po::command_line_parser(opts).options(read_version_desc).positional(p).run(), vm);

        if (vm.count("help") || !vm.count("firmware-file")) {
            cout << read_version_desc << "\n";
            printf("Usage: MSPM0_bsl_flasher read_binary_version <binary> [options]\n");
            printf("=> Example: MSPM0_bsl_flasher read_binary_version /home/foo/bar.bin\n\n");
            return 0;
        }

        bool status;
        int verbose_level = vm["verbose"].as<int>();
        const char* file_path = vm["firmware-file"].as<string>().c_str();
        uint32_t size = 0;

        auto b = BSLTool(nullptr, 0, verbose_level);
        b.open_file(file_path, size);
        std::string fw_version = b.read_file_version();
        printf("Binary: %s\nFirmware version: %s\n", file_path, fw_version.c_str());

        return 0;
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }

    return 0;
}

void print_usage()
{
    printf("Usage: MSPM0_bsl_flasher <cmd> <cmd args> [options]\n");
    printf("=> Example: MSPM0_bsl_flasher flash /dev/ttyACM0 /home/foo/bar.bin\n\n");
}

void print_version()
{
    printf("%s %s.%s.%s\n", _PROJECT_NAME_, _VERSION_MAJOR_, _VERSION_MINOR_, _VERSION_PATCH_);
}