#include <JuceHeader.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "HostServices.h"

using namespace juce;

// Debug callback wrapper to see if audioDeviceAboutToStart is called
class DiagnosticAudioCallback : public AudioIODeviceCallback
{
public:
	DiagnosticAudioCallback (AudioProcessorPlayer* player) : wrappedPlayer (player) {}
	
	void audioDeviceAboutToStart (AudioIODevice* device) override
	{
		Logger::writeToLog (">>> DiagnosticCallback: audioDeviceAboutToStart called");
		Logger::writeToLog ("    Device: " + String (device->getName()));
		Logger::writeToLog ("    Sample rate: " + String (device->getCurrentSampleRate()));
		Logger::writeToLog ("    Buffer size: " + String (device->getCurrentBufferSizeSamples()));
		
		Logger::writeToLog ("    Calling wrappedPlayer->audioDeviceAboutToStart...");
		if (wrappedPlayer)
			wrappedPlayer->audioDeviceAboutToStart (device);
		else
			Logger::writeToLog ("    ERROR: wrappedPlayer is null!");
		
		Logger::writeToLog (">>> DiagnosticCallback: audioDeviceAboutToStart completed");
	}
	
	void audioDeviceStopped() override
	{
		Logger::writeToLog (">>> DiagnosticCallback: audioDeviceStopped called");
		if (wrappedPlayer)
			wrappedPlayer->audioDeviceStopped();
	}
	
	void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
	                                       int numInputChannels,
	                                       float* const* outputChannelData,
	                                       int numOutputChannels,
	                                       int numSamples,
	                                       const AudioIODeviceCallbackContext& context) override
	{
		static int callCount = 0;
		if (callCount++ < 5)
			Logger::writeToLog (">>> DiagnosticCallback: audioDeviceIOCallback #" + String(callCount) + 
			                     " (in:" + String(numInputChannels) + " out:" + String(numOutputChannels) + 
			                     " samples:" + String(numSamples) + ")");
		
		try
		{
			if (wrappedPlayer)
				wrappedPlayer->audioDeviceIOCallbackWithContext (inputChannelData, numInputChannels,
				                                                  outputChannelData, numOutputChannels,
				                                                  numSamples, context);
			
			if (callCount <= 5)
				Logger::writeToLog (">>> DiagnosticCallback: callback completed successfully");
		}
		catch (const std::exception& e)
		{
			Logger::writeToLog (">>> EXCEPTION in audioDeviceIOCallback: " + String(e.what()));
		}
		catch (...)
		{
			Logger::writeToLog (">>> UNKNOWN EXCEPTION in audioDeviceIOCallback");
		}
	}
	
private:
	AudioProcessorPlayer* wrappedPlayer = nullptr;
};

// Forward declare createPluginFilter() which is defined in PluginProcessor.cpp
AudioProcessor* JUCE_CALLTYPE createPluginFilter();

class StandaloneMainWindow : public DocumentWindow
{
public:
	StandaloneMainWindow (AudioProcessor* processorIn, AudioHostServices* hostServices)
		: DocumentWindow ("Can Damonium",
								Colours::black,
								DocumentWindow::allButtons),
		  processor (processorIn)
	{
		setResizable (true, true);
		setResizeLimits (400, 300, 1920, 1080);

		if (processor != nullptr)
		{
			if (auto* editor = processor->createEditor())
			{
				// Pass host services to editor via processor user data
				if (hostServices != nullptr)
					processor->setPlayConfigDetails (processor->getTotalNumInputChannels(),
					                                  processor->getTotalNumOutputChannels(),
					                                  processor->getSampleRate(),
					                                  processor->getBlockSize());
				
				setContentNonOwned (editor, true);
				centreWithSize (700, 550);
			}
		}
	}

	void closeButtonPressed() override
	{
		JUCEApplication::getInstance()->systemRequestedQuit();
	}

private:
	AudioProcessor* processor = nullptr;
};

class StandalonePluginApp : public JUCEApplication, public AudioHostServices
{
public:
	StandalonePluginApp() = default;

	const String getApplicationName() override       { return "Can Damonium"; }
	const String getApplicationVersion() override    { return "1.0.0"; }
	bool moreThanOneInstanceAllowed() override             { return false; }

	// AudioHostServices implementation
	String getAudioDeviceStatus() const override
	{
		auto* device = audioDeviceManager.getCurrentAudioDevice();
		if (!device)
			return lastInitError.isEmpty() ? "No device" : ("InitErr: " + lastInitError);

		auto deviceName = device->getName();
		auto sampleRate = device->getCurrentSampleRate();
		auto bufferSize = device->getCurrentBufferSizeSamples();
		auto inputChannels = device->getActiveInputChannels().countNumberOfSetBits();
		auto outputChannels = device->getActiveOutputChannels().countNumberOfSetBits();

		return deviceName + " | SR: " + String (sampleRate, 0) + " | Block: " + String (bufferSize)
		       + " | In: " + String (inputChannels) + " | Out: " + String (outputChannels);
	}

	void showAudioSettings() override
	{
		Logger::writeToLog ("=== showAudioSettings requested ===");
		
		auto* device = audioDeviceManager.getCurrentAudioDevice();
		if (!device)
		{
			Logger::writeToLog ("No audio device available for settings");
			return;
		}

		// Log current device channel configuration
		Logger::writeToLog ("Current device: " + device->getName());
		Logger::writeToLog ("  Input channel names: " + String(device->getInputChannelNames().size()));
		for (int i = 0; i < device->getInputChannelNames().size(); ++i)
			Logger::writeToLog ("    " + String(i) + ": " + device->getInputChannelNames()[i]);
		Logger::writeToLog ("  Output channel names: " + String(device->getOutputChannelNames().size()));
		for (int i = 0; i < device->getOutputChannelNames().size(); ++i)
			Logger::writeToLog ("    " + String(i) + ": " + device->getOutputChannelNames()[i]);
		Logger::writeToLog ("  Active input channels: " + device->getActiveInputChannels().toString(2));
		Logger::writeToLog ("  Active output channels: " + device->getActiveOutputChannels().toString(2));

		// Log what devices the selector will show
		auto* deviceType = audioDeviceManager.getCurrentDeviceTypeObject();
		if (deviceType)
		{
			Logger::writeToLog ("Creating selector for device type: " + String(deviceType->getTypeName()));
			auto inputDevices = deviceType->getDeviceNames(true);
			auto outputDevices = deviceType->getDeviceNames(false);
			Logger::writeToLog ("  Available inputs: " + String(inputDevices.size()));
			for (const auto& dev : inputDevices)
				Logger::writeToLog ("    - " + dev);
			Logger::writeToLog ("  Available outputs: " + String(outputDevices.size()));
			for (const auto& dev : outputDevices)
				Logger::writeToLog ("    - " + dev);
		}

		// Create selector with channel selection enabled
		// Parameters: minInputs, maxInputs, minOutputs, maxOutputs, showMidiInputOptions, showMidiOutputOptions, showChannelsAsStereoPairs, hideAdvancedOptions
		auto* selector = new AudioDeviceSelectorComponent (audioDeviceManager, 
		                                                    0, 256,    // Allow 0-256 input channels
		                                                    0, 256,    // Allow 0-256 output channels  
		                                                    false,     // No MIDI input options
		                                                    false,     // No MIDI output options
		                                                    true,      // Show channels as stereo pairs
		                                                    false);    // Show advanced options (channel selection)
		selector->setSize (600, 600);  // Increased size for channel selectors
		
		DialogWindow::LaunchOptions options;
		options.content.setOwned (selector);
		options.dialogTitle = "Audio Settings";
		options.componentToCentreAround = mainWindow.get();
		options.dialogBackgroundColour = Colours::darkgrey;
		options.escapeKeyTriggersCloseButton = true;
		options.useNativeTitleBar = true;
		options.resizable = false;

		options.launchAsync();
		Logger::writeToLog ("Audio settings dialog launched");
	}

	String getLastInitError() const override
	{
		return lastInitError;
	}

	StringArray getInputDeviceNames() const override
	{
		Logger::writeToLog ("=== getInputDeviceNames called ===");
		Logger::writeToLog ("  Total cached input devices: " + String(availableInputDevices.size()));
		for (const auto& dev : availableInputDevices)
			Logger::writeToLog ("    - " + dev);
		return availableInputDevices;
	}

	StringArray getOutputDeviceNames() const override
	{
		Logger::writeToLog ("=== getOutputDeviceNames called ===");
		Logger::writeToLog ("  Total cached output devices: " + String(availableOutputDevices.size()));
		for (const auto& dev : availableOutputDevices)
			Logger::writeToLog ("    - " + dev);
		return availableOutputDevices;
	}

	void enumerateAudioDevices()
	{
		Logger::writeToLog ("=== Enumerating all audio devices ===");
		availableInputDevices.clear();
		availableOutputDevices.clear();

		// Get the current device type
		auto* currentDeviceType = audioDeviceManager.getCurrentDeviceTypeObject();
		if (currentDeviceType == nullptr)
		{
			Logger::writeToLog ("ERROR: No current device type!");
			return;
		}

		Logger::writeToLog ("Current device type: " + String(currentDeviceType->getTypeName()));

		// Query available input devices for this type
		auto inputDevices = currentDeviceType->getDeviceNames(true);  // true = input
		Logger::writeToLog ("Input devices available (" + String(inputDevices.size()) + "):");
		for (const auto& dev : inputDevices)
		{
			Logger::writeToLog ("  - " + dev);
			availableInputDevices.add(dev);
		}

		// Query available output devices for this type
		auto outputDevices = currentDeviceType->getDeviceNames(false);  // false = output
		Logger::writeToLog ("Output devices available (" + String(outputDevices.size()) + "):");
		for (const auto& dev : outputDevices)
		{
			Logger::writeToLog ("  - " + dev);
			availableOutputDevices.add(dev);
		}

		// Also log current selection
		AudioDeviceManager::AudioDeviceSetup setup = audioDeviceManager.getAudioDeviceSetup();
		Logger::writeToLog ("Currently selected:");
		Logger::writeToLog ("  Input:  " + setup.inputDeviceName);
		Logger::writeToLog ("  Output: " + setup.outputDeviceName);
	}

	String getCurrentInputDevice() const override
	{
		auto* device = audioDeviceManager.getCurrentAudioDevice();
		if (device)
			return device->getName();
		return "None";
	}

	String getCurrentOutputDevice() const override
	{
		auto* device = audioDeviceManager.getCurrentAudioDevice();
		if (device)
			return device->getName();
		return "None";
	}

	void setInputDevice (const String& deviceName) override
	{
		Logger::writeToLog ("=== setInputDevice requested: " + deviceName);
		// Note: In a real implementation, you would need to properly switch devices
		// This is simplified for now
	}

	void setOutputDevice (const String& deviceName) override
	{
		Logger::writeToLog ("=== setOutputDevice requested: " + deviceName);
		// Note: In a real implementation, you would need to properly switch devices
		// This is simplified for now
	}

	void initialise (const String&) override
	{
		// Set up file logging
		auto logDir = File::getSpecialLocation (File::userDocumentsDirectory)
					 .getChildFile ("Can_damonium");
		logDir.createDirectory();
		auto logFile = logDir.getChildFile ("CanDamonium.log");
		Logger::setCurrentLogger (FileLogger::createDefaultAppLogger (logDir.getFullPathName(),
										"CanDamonium.log",
										"Can Damonium Log"));
		Logger::writeToLog ("=== App start ===");
		Logger::writeToLog ("Log file: " + logFile.getFullPathName());

		processor.reset (createPluginFilter());
		Logger::writeToLog ("Processor created");

		// Set up audio player BEFORE initializing device manager
		audioPlayer.setProcessor (processor.get());
		Logger::writeToLog ("AudioPlayer processor set");
		
		// Add callback BEFORE initializing audio device (so audioDeviceAboutToStart is received)
		audioDeviceManager.addAudioCallback (&diagnosticCallback);
		Logger::writeToLog ("DiagnosticCallback added to device manager");

		// List available device types before initialization
		auto& deviceTypes = audioDeviceManager.getAvailableDeviceTypes();
		Logger::writeToLog ("Available device types: " + String(deviceTypes.size()));
		for (auto* type : deviceTypes)
		{
			if (type)
				Logger::writeToLog ("  - " + String(type->getTypeName()));
		}

		// Now initialize - this will trigger audioDeviceAboutToStart
		// Use nullptr to let JUCE choose the best available device type (typically WASAPI on Windows)
		lastInitError = audioDeviceManager.initialise (2, 2, nullptr, true);
		if (lastInitError.isNotEmpty())
			Logger::writeToLog ("AudioDeviceManager init error: " + lastInitError);
		else
			Logger::writeToLog ("AudioDeviceManager initialized successfully");
		
		// Check what device type was selected
		auto* currentType = audioDeviceManager.getCurrentDeviceTypeObject();
		if (currentType)
			Logger::writeToLog ("Current device type: " + String(currentType->getTypeName()));

		auto* device = audioDeviceManager.getCurrentAudioDevice();
		if (device)
		{
			Logger::writeToLog ("Audio device: " + device->getName());
			Logger::writeToLog ("Sample rate: " + String (device->getCurrentSampleRate()));
			Logger::writeToLog ("Buffer size: " + String (device->getCurrentBufferSizeSamples()));
			Logger::writeToLog ("Input channels: " + String (device->getActiveInputChannels().countNumberOfSetBits()));
			Logger::writeToLog ("Output channels: " + String (device->getActiveOutputChannels().countNumberOfSetBits()));
		}
		else
		{
			Logger::writeToLog ("ERROR: No audio device available!");
		}

		// Enumerate all available audio devices
		enumerateAudioDevices();

		// Prefer Focusrite devices if available (avoids silent default inputs like S/PDIF)
		auto pickPreferred = [] (const StringArray& devices, const String& token1, const String& token2) -> String
		{
			for (const auto& dev : devices)
				if (dev.containsIgnoreCase (token1) && dev.containsIgnoreCase (token2))
					return dev;
			for (const auto& dev : devices)
				if (dev.containsIgnoreCase (token1))
					return dev;
			return {};
		};

		auto preferredInput = pickPreferred (availableInputDevices, "Focusrite", "Analogue");
		auto preferredOutput = pickPreferred (availableOutputDevices, "Focusrite", "USB");

		if (preferredInput.isNotEmpty() || preferredOutput.isNotEmpty())
		{
			auto setup = audioDeviceManager.getAudioDeviceSetup();
			if (preferredInput.isNotEmpty())
				setup.inputDeviceName = preferredInput;
			if (preferredOutput.isNotEmpty())
				setup.outputDeviceName = preferredOutput;
			setup.useDefaultInputChannels = false;
			setup.useDefaultOutputChannels = false;
			setup.inputChannels.clear();
			setup.outputChannels.clear();
			setup.inputChannels.setRange (0, 2, true);
			setup.outputChannels.setRange (0, 2, true);

			auto err = audioDeviceManager.setAudioDeviceSetup (setup, true);
			if (err.isNotEmpty())
				Logger::writeToLog ("ERROR: setAudioDeviceSetup failed: " + err);
			else
				Logger::writeToLog ("Audio device switched to preferred Focusrite I/O");

			// Re-enumerate after switching
			enumerateAudioDevices();
		}

		// Store host services pointer BEFORE creating window so editor can access it
		hostServicesPtr = this;

		// Pass host services to the window (which will pass to editor)
		mainWindow = std::make_unique<StandaloneMainWindow> (processor.get(), this);
		
		mainWindow->setVisible (true);
		DBG ("Window shown, initialization complete");
	}

	void shutdown() override
	{
		Logger::writeToLog ("=== App shutdown ===");
		hostServicesPtr = nullptr;
		
		if (mainWindow)
			mainWindow = nullptr;

		audioDeviceManager.removeAudioCallback (&diagnosticCallback);
		audioPlayer.setProcessor (nullptr);
		processor = nullptr;
		Logger::setCurrentLogger (nullptr);
	}

	void systemRequestedQuit() override
	{
		DBG ("=== Quit requested ===");
		quit();
	}

	// Static accessor for host services (only valid during app lifetime)
	static AudioHostServices* getHostServices() { return hostServicesPtr; }

private:
	std::unique_ptr<AudioProcessor> processor;
	AudioDeviceManager audioDeviceManager;
	AudioProcessorPlayer audioPlayer;
	DiagnosticAudioCallback diagnosticCallback {&audioPlayer};
	std::unique_ptr<StandaloneMainWindow> mainWindow;
	String lastInitError;
	StringArray availableInputDevices;
	StringArray availableOutputDevices;
	
	static inline AudioHostServices* hostServicesPtr = nullptr;
};

START_JUCE_APPLICATION (StandalonePluginApp)

// Provide access to host services for plugin editor
extern "C" AudioHostServices* getHostServicesInstance()
{
	return StandalonePluginApp::getHostServices();
}
