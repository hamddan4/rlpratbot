# -*- coding: utf-8 -*-

import serial
import time
import telepot
from telepot.loop import MessageLoop
from telepot.namedtuple import ReplyKeyboardMarkup, KeyboardButton, ReplyKeyboardRemove
import struct 


print('hello')

test = False

codes = [u'↶', u'↑', u'↷', u'✋', u'↵', u'↓', u'↳', u'-', u'+']
dir_codes = {c: n for n, c in enumerate(codes)}

if(not test):
    ser = serial.Serial(port='COM6', baudrate=9600)

def handle(msg):
    content_type, chat_type, chat_id = telepot.glance(msg)
    print(content_type, chat_type, chat_id,msg['from']['first_name']+msg['from']['last_name'])



    if content_type == 'location':
        message = 'Sending soda to:\n'
        print(msg['location']['latitude'])
        message += 'latitude: ' + str(msg['location']['latitude']) + '\n'
        message += 'longitude: ' + str(msg['location']['longitude'])
        
        bot.sendMessage(chat_id, message)

        bot.sendMessage(chat_id, "Command sent")
        if(not test):
            ser.write(bytes([255]))
            ser.write(struct.pack("f",msg['location']['latitude']))
            ser.write(struct.pack("f",msg['location']['longitude']))
        
        
    elif content_type == 'text':
        if  msg['text'] == "/start":
            
            message = 'Hello ' + msg['from']['first_name'] + ' ' + msg['from']['last_name'] + '\n'
            message += 'I\'m RAT waiter, a bot that will be used for controlling me also bring you a SODA.\n \n'
            message += 'These are the commands that I can now handle:\n'
            message += '/start welcome and information about me\n'
            message += '/show_commands Show the keyboard to move the me\n'
            message += '/hide_commands Hide the keyboard to move the me\n'
            
            bot.sendMessage(chat_id, message)
            
        elif msg['text'] == '/show_commands':
            message = "Showing it"
            resposta=ReplyKeyboardMarkup(keyboard=[\
            [KeyboardButton(text="↶"), KeyboardButton(text="↑"), KeyboardButton(text="↷")],\
            [KeyboardButton(text="-"), KeyboardButton(text="✋"), KeyboardButton(text="+")],\
            [KeyboardButton(text="↵"), KeyboardButton(text="↓"), KeyboardButton(text="↳")]\
            ])
            bot.sendMessage(chat_id, message, reply_markup = resposta)
            
        elif msg['text'] == '/hide_commands':
            message = "Hiding it"
            resposta = ReplyKeyboardRemove(remove_keyboard = True)
            bot.sendMessage(chat_id, message, reply_markup = resposta)
        elif msg['text'] in codes:
            bot.sendMessage(chat_id, "Command sent")
            if(not test):
                ser.write(bytes([dir_codes[msg['text']]]))
        else:
            bot.sendMessage(chat_id, "Message not recognized")
            
            
        
bot = telepot.Bot('347200641:AAHCedET7_Bf-qaClV-In1FCp8xID9iflYQ')
print(bot.getMe())

MessageLoop(bot, handle).run_as_thread()
print('Listening ...')

while 1:
	time.sleep(10)
