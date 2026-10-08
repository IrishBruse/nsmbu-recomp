#include "../runtime.h"

HLE(snd_user, AXFXGetHooks) {}
HLE(snd_user, AXFXSetHooks) {}

HLE(snd_user, AXFXChorusExpGetMemSize) { ret(c, 64); }
HLE(snd_user, AXFXChorusExpInit) { ret(c, 1); }
HLE(snd_user, AXFXChorusExpSettingsUpdate) {}
HLE(snd_user, AXFXChorusExpShutdown) {}
HLE(snd_user, AXFXChorusExpCallback) {}

HLE(snd_user, AXFXDelayExpGetMemSize) { ret(c, 64); }
HLE(snd_user, AXFXDelayExpInit) { ret(c, 1); }
HLE(snd_user, AXFXDelayExpSettings) {}
HLE(snd_user, AXFXDelayExpSettingsUpdate) {}
HLE(snd_user, AXFXDelayExpShutdown) {}
HLE(snd_user, AXFXDelayExpCallback) {}

HLE(snd_user, AXFXReverbHiExpGetMemSize) { ret(c, 64); }
HLE(snd_user, AXFXReverbHiExpInit) { ret(c, 1); }
HLE(snd_user, AXFXReverbHiExpSettings) {}
HLE(snd_user, AXFXReverbHiExpSettingsUpdate) {}
HLE(snd_user, AXFXReverbHiExpShutdown) {}
HLE(snd_user, AXFXReverbHiExpCallback) {}

HLE(snd_user, AXFXReverbStdExpGetMemSize) { ret(c, 64); }
HLE(snd_user, AXFXReverbStdExpInit) { ret(c, 1); }
HLE(snd_user, AXFXReverbStdExpSettings) {}
HLE(snd_user, AXFXReverbStdExpSettingsUpdate) {}
HLE(snd_user, AXFXReverbStdExpShutdown) {}
HLE(snd_user, AXFXReverbStdExpCallback) {}
