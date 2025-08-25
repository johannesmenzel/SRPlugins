#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_NAME  "AutoLeveler"
#define DISTRHO_PLUGIN_URI   "https://github.com/johannesmenzel/SRPlugins/lv2-components/AutoLeveler"
#define DISTRHO_PLUGIN_LV2_CATEGORY "lv2:UtilityPlugin"

#define DISTRHO_PLUGIN_NUM_INPUTS       2
#define DISTRHO_PLUGIN_NUM_OUTPUTS      2
#define DISTRHO_PLUGIN_IS_RT_SAFE       1
#define DISTRHO_PLUGIN_HAS_UI           0
#define DISTRHO_PLUGIN_WANT_MIDI_INPUT  0
#define DISTRHO_PLUGIN_WANT_MIDI_OUTPUT 0
#define DISTRHO_PLUGIN_WANT_PROGRAMS    0

enum Parameters {
    kThreshPeak,
    kThreshRMS,
    kPreGain,
    kPan,
    kParametersCount
};

#endif