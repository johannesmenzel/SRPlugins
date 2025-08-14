#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_NAME  "AutoLeveler"
#define DISTRHO_PLUGIN_URI   "https://github.com/johannesmenzel/SRPlugins/lv2-components/AutoLeveler"

#define DISTRHO_PLUGIN_NUM_INPUTS   2
#define DISTRHO_PLUGIN_NUM_OUTPUTS  2
#define DISTRHO_PLUGIN_IS_RT_SAFE   1

enum Parameters {
    kGain,
    kParameterCount
};

#endif