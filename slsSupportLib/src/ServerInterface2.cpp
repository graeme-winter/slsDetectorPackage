#include "ServerInterface2.h"
#include <cassert>
#include <cstring>
#include <sstream>
namespace sls {

int ServerInterface2::sendResult(int ret, void *retval, int retvalSize,
                                 char *mess) {

    write(&ret, sizeof(ret));
    if (ret == defs::FAIL) {
        if (mess != nullptr) {
            write(mess, MAX_STR_LENGTH);
        } else {
            FILE_LOG(logERROR) << "No error message provided for this "
                                  "failure. Will mess up TCP\n";
        }
    } else {
        write(retval, retvalSize);
    }
    return ret;
}

} // namespace sls
