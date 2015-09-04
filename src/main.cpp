
#include "Config.hpp"
#include "Portal.hpp"

int main(int argc, char* argv[])
{
    csocks::Config::mutableInstance()->init(argc, argv);
    csocks::Portal portal;
    portal.run();
    return 0;
}
