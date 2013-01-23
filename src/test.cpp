#include "fluent_cpp.h"
#include <sstream>
using namespace fluent;

int main(int argc, const char * argv[])
{
    int port = 24224;
    if( argc > 1 ) {
        ::std::stringstream strm;
        strm << argv[1];
        strm >> port;
    }

    Logger logger("fluent.test", "0.0.0.0", port);
    logger.log("", "from", "userA", "to", "userB");
    return 0;
}
