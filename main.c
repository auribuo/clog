#define CLOG_IMPLEMENTATION // Needed: pull in implementation
#define CLOG_ENABLE_CTX   // Needed to use contexts
#define CLOG_STRIP_PREFIX // Optional: Used to strip prefixes
#include "clog.h"

typedef struct {
    const char *name;
    int age;
    int fav_num;
    const char *address;
} Person;

int main(void) {
    set_log_level(debug);    // Set the log level (default, info)
    set_color_enabled(true); // Enable colors. In unix you can use set_color_enabled_auto() to auto enable colors for ttys

    Person p = {
        .name = "Max Mustermann",
        .age = 22,
        .fav_num = 42,
        .address = "Via Roma 1"};

    // Filters can disable certain contexts. The filter syntax looks like this: [log|nolog]:[<context>|all].
    // Multiple filters can be separated by ';'
    parse_log_filter("nolog:main");
    parse_log_filter_env(); // Filters can be supplied using the CLOG_FILTER env variable. The syntax is the same

    LOG_INFO("This is a test log");
    LOG_INFO("This is a test log with some info added", LOG_ARG_S("name", p.name));
    LOG_WARNING("You can use shorthand args", LOG_ARG_SS(p.address));
    LOG_ERROR("Oh, no!", LOG_ARG_UB("fav_bin_num", p.fav_num)); // Numbers can be formatted differently
    LOG_DEBUG("Debug includes the file and line");
    LOG_INFO_CTX("main", "You can also include contexts to logs"); // Hidden because of filter
}
