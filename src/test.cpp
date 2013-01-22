#include "fluent_cpp.h"
#include <errno.h>
using namespace fluent;

int main() {
    errno = 0;
    Logger logger("test", "0.0.0.0", 24224);
    logger.log("fluent", "key", "value");
    return 0;
}
