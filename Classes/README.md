# SRClasses
These are shared classes for SRPlugins, either DSP classes without third party dependencies or specific classes for IPlug2.
# Included classes
Besides others: SRConstants which holds specific variables and classes used in all (future) SR plugins.
## DSP
DSP classes mostly without dependencies.
### SRBuffer
Simple multichannel buffer, depends on WDL (Ptrlist and Typedbuf)
### SRGain
Gain and pan classes with gain smoother
### SRFilters
Mainly biquad filters and some others, based on Earlevel biquad classes and some other resources.
### SRDynamics
Dynamics classes including compressors, limiters, gate, deesser and so on. More or less based on chunkware simple classes.
### SRSaturation
Some waveshapers.
## Graphics
### SRControls
Specific IPlug2 controls inherited from IControl
## Utils
### SRHelpers
Some helpers for unit conversion and so on. Without dependencies.
