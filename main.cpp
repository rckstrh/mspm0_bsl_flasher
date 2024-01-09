#include <iostream>
#include "bsl_tool.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace std;

void print_usage();
int flash(po::variables_map &vm, po::parsed_options &parsed);   // flash subcommand

int main(int argc, char** argv) {
    try {
        // main options/commands
        po::options_description main_desc("Main options");
        main_desc.add_options()
            ("help,h", "produce help message")
            ("command", po::value<string>(), "command (flash, reset, read_binary_version)")
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

        // handle subcommands
        if(vm.count("command")) {
            string cmd = vm["command"].as<string>();
            if(cmd == "flash"){
                return flash(vm, parsed);
            } else if(cmd == "reset"){
                // TODO
                return 0;
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
        po::options_description flash_desc("flash options");
        flash_desc.add_options()
            ("help,h", "produce help message")
            ("serial-port,p", po::value<string>(), "serial port (e.g. /dev/ttyACM0)")
            ("firmware-file,i", po::value<string>(), "firmware file")
            ("enter-bsl", po::value<bool>()->default_value(true), "enter BSL mode via GPIOs (default: true)")
        ;

        po::positional_options_description p;
        p.add("serial-port", 1);
        p.add("firmware-file", 2);

        // erase command name
        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());

        // reparse
        po::store(po::command_line_parser(opts).options(flash_desc).positional(p).run(), vm);

        if (vm.count("help") || !vm.count("serial-port") || !vm.count("firmware-file")) {
            cout << flash_desc << "\n";
            printf("Usage: MSPM0_bsl_flasher flash <serial> <binary> [options]\n");
            printf("=> Example: MSPM0_bsl_flasher /dev/ttyACM0 /home/foo/bar.bin\n\n");
            return 0;
        }

        bool status;
        bool enter_bsl_gpio = vm["enter-bsl"].as<bool>();
        const char* serial_path = vm["serial-port"].as<string>().c_str();
        const char* file_path = vm["firmware-file"].as<string>().c_str();
        uint32_t size = 0;

        auto b = BSLTool(serial_path, enter_bsl_gpio);
        b.open_file(file_path, size);
        std::string fw_version = b.read_file_version();
        printf("Using serial %s to flash %s\nFirmware version:%s\n", serial_path, file_path, fw_version.c_str());

        if(enter_bsl_gpio) {
            printf("Entering BSL mode\n");
            status = b.enter_bsl();
            if(!status) {
                printf("Could not enter BSL mode. Stopping...\n");
                return status;
            }
        }

        status = b.flash_image(file_path);
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

void print_usage()
{
    printf("Usage: MSPM0_bsl_flasher <cmd> <cmd args> [options]\n");
    printf("=> Example: MSPM0_bsl_flasher flash /dev/ttyACM0 /home/foo/bar.bin\n\n");
}