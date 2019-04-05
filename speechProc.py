import speech_recognition as sr
import pyaudio
import wave
import requests
import json
import pygame
import RPi.GPIO as GPIO
import time
import os
from xml.etree import ElementTree
import random

############################################ 語意分析 ####################################
def getluis(query):
    headers = {
        # Request headers
        'Ocp-Apim-Subscription-Key':'dceb60a4d77f4104a21d3a2cef5cb6e3',
    }

    params ={
        # Query parameter
        'q': query,
        # Optional request parameters, set to default values
        'timezoneOffset': '0',
        'verbose': 'false',
        'spellCheck': 'false',
        'staging': 'false',
    }

    try:
        r = requests.get('https://westus.api.cognitive.microsoft.com/luis/v2.0/apps/8aeb3e0e-3e22-4495-88d8-e0d73eaa2f0e',headers=headers, params=params)
        print(type(r.json()))
        return r.json()
        #text = json.loads(r.json())
        #prin(text)

    except Exception as e:
        print("[Errno {0}] {1}".format(e.errno, e.strerror))

######################################### record ###################################
def record(wav_output_filename):
    form_1 = pyaudio.paInt16 # 16-bit resolution
    chans = 1 # 1 channel
    samp_rate = 44100 # 44.1kHz sampling rate
    chunk = 882 # 2^12 samples for buffer
    record_secs = 4 # seconds to record
    dev_index = 2 # device index found by p.get_device_info_by_index(ii)

    audio = pyaudio.PyAudio() # create pyaudio instantiation

    # create pyaudio stream
    stream = audio.open(format = form_1,rate = samp_rate,channels = chans, \
                        input = True, frames_per_buffer=chunk)
    #input_device_index = dev_index,
    print("recording")
    frames = []

    # loop through stream and append audio chunks to frame array
    for ii in range(0,int((samp_rate/chunk)*record_secs)):
        data = stream.read(chunk)
        frames.append(data)

    print("finished recording")
    # stop the stream, close it, and terminate the pyaudio instantiation
    stream.stop_stream()
    stream.close()
    audio.terminate()

    # save the audio frames as .wav file
    wavefile = wave.open(wav_output_filename,'wb')
    wavefile.setnchannels(chans)
    wavefile.setsampwidth(audio.get_sample_size(form_1))
    wavefile.setframerate(samp_rate)
    wavefile.writeframes(b''.join(frames))
    wavefile.close()


############################################ 語音辨識 ####################################
def google_stt(wav_output_filename):
    #obtain audio from the microphone
    r=sr.Recognizer() 

    with sr.AudioFile(wav_output_filename) as source:
         audio = r.record(source)
    try:
        print("Google Speech Recognition thinks you said:")
        txt = r.recognize_google(audio, language="zh-TW")
        print(r.recognize_google(audio, language="zh-TW"))
        return txt
    except sr.UnknownValueError:
        print("Google Speech Recognition could not understand audio")
        return "0"
    except sr.RequestError as e:
        print("No response from Google Speech Recognition service: {0}".format(e))
        return "0"

############################################ story telling: tts ####################################
# This code is required for Python 2.7
#try: input = raw_input
#except NameError: pass

class TextToSpeech(object):
    def __init__(self, file):
        self.subscription_key = "215583552b514853b055bfaa05a8431a"
        #self.tts = input("What would you like to convert to speech: ")
        with open(file,'r') as file:
            text = file.read()
            print ('open the txt...')#.decode('utf-8')
            self.tts = text#.decode('utf-8')
            self.timestr = "TTS_Azure"
        #self.timestr = time.strftime("%Y%m%d-%H%M")
        self.access_token = None

    '''
    The TTS endpoint requires an access token. This method exchanges your
    subscription key for an access token that is valid for ten minutes.
    '''
    def get_token(self):
        fetch_token_url = "https://westus.api.cognitive.microsoft.com/sts/v1.0/issueToken"
        headers = {
            'Ocp-Apim-Subscription-Key': self.subscription_key
        }
        response = requests.post(fetch_token_url, headers=headers)
        self.access_token = str(response.text)

    def save_audio(self):
        base_url = 'https://westus.tts.speech.microsoft.com/'
        path = 'cognitiveservices/v1'
        constructed_url = base_url + path
        headers = {
            'Authorization': 'Bearer ' + self.access_token,
            'Content-Type': 'application/ssml+xml',
            'X-Microsoft-OutputFormat': 'riff-24khz-16bit-mono-pcm',
            'User-Agent': 'YOUR_RESOURCE_NAME'
        }
        xml_body = ElementTree.Element('speak', version='1.0')
        xml_body.set('{http://www.w3.org/XML/1998/namespace}lang', 'en-us')
        voice = ElementTree.SubElement(xml_body, 'voice')
        voice.set('{http://www.w3.org/XML/1998/namespace}lang', 'en-US')
        voice.set('name', 'Microsoft Server Speech Text to Speech Voice (en-US, Guy24KRUS)')
        voice.set(
        'name', 'Microsoft Server Speech Text to Speech Voice (zh-TW, Yating, Apollo)')
        voice.text = self.tts
        body = ElementTree.tostring(xml_body)

        response = requests.post(constructed_url, headers=headers, data=body)
        '''
        If a success response is returned, then the binary audio is written
        to file in your working directory. It is prefaced by sample and
        includes the date.
        '''
        if response.status_code == 200:
            with open(storyfile + '.wav', 'wb') as audio:
                audio.write(response.content)
                print("\nStatus code: " + str(response.status_code) + "\nYour TTS is ready for playback.\n")
        else:
            print("\nStatus code: " + str(response.status_code) + "\nSomething went wrong. Check your subscription key and headers.\n")
'''
def main():
    
    record()
    txt = google_stt()
    if(txt != "0"):
        getluis(txt)


    while (1):
        if :
            pass
main()
'''

if __name__ == "__main__":

    GPIO.setmode(GPIO.BCM)
    GPIO.setup(18, GPIO.IN)#, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(17, GPIO.IN)#, pull_up_down=GPIO.PUD_UP)

    file = 'command.wav'

    #prev_state = 0
    stop = False
    pygame.mixer.init()
    pygame.mixer.music.set_volume(1)
    while(True):
        time.sleep(1)
        print('waiting for command ... ...')

        input_record = GPIO.input(18)
        input_alarm = GPIO.input(17)

        print(GPIO.input(17))
        
        if(input_alarm==1):
            pygame.mixer.music.load('alarm.wav')
            pygame.mixer.music.play()
            pygame.mixer.music.fadeout(5000)
            while pygame.mixer.music.get_busy() == True:
                continue
            if(pygame.mixer.music.get_busy() == False):
                print('alarm stop.')
            
        
        if(input_record==1):
            record(file)
            txt = google_stt(file)
            if(txt != "0"):
                txtdict = getluis(txt)
                if(txtdict['topScoringIntent']['intent'] == '日程.删除'):
                    print('刪除鬧鐘...')
                    #if(txtdict['entities'][1]['type'] == '時點'):
                     #   print('刪除', txtdict['entities'][1]['entity'],'的鬧鐘')
                elif(txtdict['topScoringIntent']['intent'] == '日程.添加日程'):
                    print('添加鬧鐘...')
                    #if(txtdict['entities'][1]['type'] == '時點'):
                     #   print('加入', txtdict['entities'][1]['entity'],'的鬧鐘')
                elif(txtdict['topScoringIntent']['intent'] == '故事'):
                    print('開始講故事...')
                    story_num = random.randint(1,10)
                    storyfile = 'story/' + str(story_num) + '.txt'
                    app = TextToSpeech(storyfile)
                    app.get_token()
                    app.save_audio()
                    
                    pygame.mixer.music.load(storyfile + '.wav')
                    pygame.mixer.music.play()
                    while pygame.mixer.music.get_busy() == True:
                        continue
                elif(txtdict['topScoringIntent']['intent'] == '笑話'):
                    print('開始講笑話...')
                    story_num = random.randint(1,10)
                    storyfile = 'joke/' + str(story_num) + '.txt'
                    app = TextToSpeech(storyfile)
                    app.get_token()
                    app.save_audio()
                    
                    pygame.mixer.music.load(storyfile + '.wav')
                    pygame.mixer.music.play()
                    while pygame.mixer.music.get_busy() == True:
                        continue
                else:
                    print('do not know what you saying, please try again...')
                    '''storyfile = 'notknow.txt'
                    app = TextToSpeech(storyfile)
                    app.get_token()
                    app.save_audio()'''
                    pygame.mixer.music.load('notknow.txt.wav')
                    pygame.mixer.music.play()
                    while pygame.mixer.music.get_busy() == True:
                        continue
            else:
                print('do not know what you saying, please try again...')
                pygame.mixer.music.load('notknow.txt.wav')
                pygame.mixer.music.play()
                while pygame.mixer.music.get_busy() == True:
                    continue
            time.sleep(1)
