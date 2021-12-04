#define PLUG_NAME "SRDynamicsControl"
#define PLUG_MFR "SRPlugins"
#define PLUG_VERSION_HEX 0x00000001
#define PLUG_VERSION_STR "0.0.1"
#define PLUG_UNIQUE_ID 'SrDC'
#define PLUG_MFR_ID 'SrPl'
#define PLUG_URL_STR "https://github.com/johannesmenzel/SRPlugins"
#define PLUG_EMAIL_STR "spam@me.com"
#define PLUG_COPYRIGHT_STR "Copyright 2021 Johannes Menzel"
#define PLUG_CLASS_NAME SRDynamicsControl

#define BUNDLE_NAME "SRDynamicsControl"
#define BUNDLE_MFR "SRPlugins"
#define BUNDLE_DOMAIN "com"

#define SHARED_RESOURCES_SUBPATH "SRDynamicsControl"

#define PLUG_CHANNEL_IO "1-1 2-2"

#define PLUG_LATENCY 0
#define PLUG_TYPE 0
#define PLUG_DOES_MIDI_IN 0
#define PLUG_DOES_MIDI_OUT 0
#define PLUG_DOES_MPE 0
#define PLUG_DOES_STATE_CHUNKS 0
#define PLUG_HAS_UI 1
#define PLUG_WIDTH 300
#define PLUG_HEIGHT 200
#define PLUG_FPS 60
#define PLUG_SHARED_RESOURCES 0
#define PLUG_HOST_RESIZE 0

#define AUV2_ENTRY SRDynamicsControl_Entry
#define AUV2_ENTRY_STR "SRDynamicsControl_Entry"
#define AUV2_FACTORY SRDynamicsControl_Factory
#define AUV2_VIEW_CLASS SRDynamicsControl_View
#define AUV2_VIEW_CLASS_STR "SRDynamicsControl_View"

#define AAX_TYPE_IDS 'IEF1', 'IEF2'
#define AAX_TYPE_IDS_AUDIOSUITE 'IEA1', 'IEA2'
#define AAX_PLUG_MFR_STR "Acme"
#define AAX_PLUG_NAME_STR "SRDynamicsControl\nIPEF"
#define AAX_PLUG_CATEGORY_STR "Effect"
#define AAX_DOES_AUDIOSUITE 1

#define VST3_SUBCATEGORY "Fx"

#define APP_NUM_CHANNELS 2
#define APP_N_VECTOR_WAIT 0
#define APP_MULT 1
#define APP_COPY_AUV3 0
#define APP_SIGNAL_VECTOR_SIZE 64

#define ROBOTO_FN "Roboto-Regular.ttf"
