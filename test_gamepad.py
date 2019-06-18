#import evdev
from evdev import InputDevice, categorize, ecodes
import time

gamepad = InputDevice('/dev/input/event4')
print(gamepad)


#evdev takes care of polling the controller in a loop


for event in gamepad.read_loop():
    print event.code, event.type, event.value

    if event.code == 17:
        if event.value == -1:
            print "expo++"
        elif event.value == 1:
            print "expo--"

    if event.code == 308:
        if event.value == 0:
            print "apert++"
    if event.code == 304:
        if event.value == 0:
            print "apert--"


    #print(categorize(event))


# ['code', 'sec', 'timestamp', 'type', 'usec', 'value']
'''
'active_keys', 'capabilities', 'close', 'erase_effect', 'fd', 'ff_effects_count', 
'fileno', 'fn', 'grab', 'grab_context', 'info', 'leds', 'name', 'need_write', 
'path', 'phys', 'read', 'read_loop', 'read_one', 'repeat', 
'set_led', 'ungrab', 'uniq', 'upload_effect', 
'version', 'write', 'write_event']
'''



