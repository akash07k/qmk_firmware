#pragma once

#undef CONNECTED_IDLE_TIME
#define CONNECTED_IDLE_TIME 600

#undef P2P4G_CELAR_MASK
// Bit 0 is USB-A and bit 1 is USB-C; a full reset must clear both.
#define P2P4G_CELAR_MASK 0x03
