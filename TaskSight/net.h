#pragma once

struct NetSpeed {
    double downloadBps = 0.0;
    double uploadBps = 0.0;
};

NetSpeed GetNetworkSpeed();