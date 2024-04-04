# Centroid, Roll-off & balance Visualizer

This is a simple real-time visualizer of some audio signal features such as spectral centroid, roll-on and roll-off frequency, stereo balance and RMS amplitude. It is developed using JUCE framework (https://juce.com) therefore JUCE has to be installed for compiling this project. It can work as a standalone application or as a VST plugin.

Building the application:
1. Open ChordVisualizer.jucer file using the Projucer (the IDE tool for creating and managing JUCE projects).
2. In "File -> Global paths" window choose your OS and correct "Path to JUCE" and "JUCE modules" according to your JUCE directories.
3. Select the exporter that your need, press "Save and open in IDE" button.
4. Build the solution.

The user interface is rather easy.

Spectral roll-off frequency is the frequency below which a specified percentage of the total spectral energy lies. It is common to use 85%, but the user can set another percentage. Similarly, roll-on frequency percentage also can be specified.
The range between the roll-on and roll-off frequency for each channel is displayed as a rounded rectangle with the higher and lower side height corresponding to the roll-off and roll-on respectively. The gradient fill is used to draw it, and the brightest line height is corresponding to the spectral centroid frequency. As it doesn't equal to the 50%-energy frequency, then there can be cases when this frequency is outside the range between the roll-on and roll-off. In these cases the pale rectangle is displayed with the permanent color fill and the centroid line is located outside of it.

The balance and RMS are displayed as a "ray": it's center correspondes to balance and it's width (it can be spoken of as a "diameter") is proportional to the RMS value.

All values are smoothed before displaying. Smoothing coefficients can be chosen by the user: the more the coefficient, the slower the corresponding displaying parameters are changed.

Comments, bug reports, questions are appreciated.

#JUCE
#Audio
#DSP
#Digital Signal Processing
#Plugin
#Plug-in
#C++