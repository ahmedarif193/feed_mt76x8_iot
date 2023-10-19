#include <main.h>

void display_command_usage(char* progname) {
    fprintf(stderr, 
        "\n"
        "Usage: %s gpiomux get\n"
        "\tList the current GPIO muxing configuration\n\n"
        "Usage: %s gpiomux set <GPIO group> <mux setting>\n"
        "\tSet the GPIO muxing for the specified GPIO signal group\n\n"
        "Usage: %s refclk get\n"
        "\tDisplay the current refclk setting\n\n"
        "Usage: %s refclk set <frequency (MHz)>\n"
        "\tSet the refclk to the specified frequency if possible\n\n",
        progname, progname, progname, progname
    );
}

int handle_gpiomux_operations(int argc, char **argv) {
    if (open_gpio_mapping() == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    int operation_status = EXIT_FAILURE;

    if (argc >= 5 && !strcmp(argv[2], "set")) {
        operation_status = configure_gpiomux(argv[3], argv[4]);
    } else if (argc >= 3 && !strcmp(argv[2], "get")) {
        operation_status = display_gpiomux();
    } else {
        display_command_usage(*argv);
    }

    close_gpio_mapping();
    return operation_status;
}

int handle_refclk_operations(int argc, char **argv) {
    if (open_reference_clock_mapping() == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    int operation_status = EXIT_FAILURE;

    if (argc >= 4 && !strcmp(argv[2], "set")) {
        operation_status = set_reference_clock(atoi(argv[3]));
    } else if (argc >= 3 && !strcmp(argv[2], "get")) {
        operation_status = get_reference_clock();
    } else {
        display_command_usage(*argv);
    }

    close_reference_clock_mapping();
    return operation_status;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        display_command_usage(*argv);
        return EXIT_FAILURE;
    }

    if (!strcmp(argv[1], "gpiomux") || !strcmp(argv[1], "pinmux")) {
        return handle_gpiomux_operations(argc, argv);
    } else if (!strcmp(argv[1], "refclk")) {
        return handle_refclk_operations(argc, argv);
    } else {
        display_command_usage(*argv);
        return EXIT_FAILURE;
    }
}
