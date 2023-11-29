import speech_recognition as sr

# Initialize the recognizer
r = sr.Recognizer()

# Define the microphone as the audio source
mic = sr.Microphone()

# Adjust microphone sensitivity (optional)
mic.energy_threshold = 5000

while True:

    # Set the language to Vietnamese
    with mic as source:
#        print(audio)
        try:
            # Recognize speech using the Vietnamese language
            r.adjust_for_ambient_noise(source)
    #        print("Say something...")
            audio = r.listen(source)
            text = r.recognize_google(audio, language='vi-VN') 
            print(text,end=' ')
        except sr.UnknownValueError:
            print('.',end=' ')
        except sr.RequestError as e:
            print("Error occurred:", str(e))