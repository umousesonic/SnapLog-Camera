import os
os.environ['KIVY_NO_FILELOG'] = '1'  # eliminate file log
os.environ['KIVY_NO_CONSOLELOG'] = '1'  # eliminate console log

from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.textinput import TextInput
from kivy.uix.button import Button
from kivy.uix.widget import Widget
from kivy.uix.textinput import TextInput
import os
from datetime import datetime
from share_queue import cmd_queue        
import subprocess

from threading import Thread
from packet import Cmd, Cmdpacket

class PictureCaptureApp(App):

    def build(self):
        # Overall vertical box
        main_layout = BoxLayout(orientation='vertical')

        # Forth box: Horizontal box with one Button
        buttons_layout = BoxLayout(orientation='horizontal', size_hint_y = 0.2)
        power_off_button = Button(text='Power Off')
        # TODO
        # we need to create on click event
        power_off_button.bind(on_press=self.power_off)

        buttons_layout.add_widget(power_off_button)
        main_layout.add_widget(buttons_layout)


        # Label for displaying validation messages
        self.message_label = Label(size_hint_y=None, font_size=20)
        main_layout.add_widget(self.message_label)

        # First box: Label for setting time
        setting_label = Label(text='Setting time for auto capturing...', font_size=40)
        main_layout.add_widget(setting_label)

        # Second box: Centered horizontal box with TextInput and 'min' Label
        timing_layout = BoxLayout(orientation='horizontal', size_hint_y=None, height=50, spacing=10)
        # Use a dummy widget with a flexible size_hint_x to push your widgets to the center
        dummy_left = Widget(size_hint_x=1)
        self.timing_input_min = TextInput(multiline=False, size_hint_x=None, width=200)
        timing_label_min = Label(text='min', size_hint_x=None, width=50)
        self.timing_input_sec = TextInput(multiline=False, size_hint_x=None, width=200)
        timing_label_sec = Label(text='sec', size_hint_x=None, width=50)
        dummy_right = Widget(size_hint_x=1)
        # Add the dummy widgets and the input + label
        timing_layout.add_widget(dummy_left)
        timing_layout.add_widget(self.timing_input_min)
        timing_layout.add_widget(timing_label_min)
        timing_layout.add_widget(self.timing_input_sec)
        timing_layout.add_widget(timing_label_sec)

        timing_layout.add_widget(dummy_right)
        main_layout.add_widget(timing_layout)

        # Third box: Horizontal box with two Buttons
        buttons_layout = BoxLayout(orientation='horizontal', size_hint_y = 0.1)
        set_time_button = Button(text='Set Time')
        set_time_button.bind(on_press=self.set_time)

        create_video_button = Button(text='Create Video')
        create_video_button.bind(on_press=self.create_video)

        buttons_layout.add_widget(set_time_button)
        buttons_layout.add_widget(create_video_button)
        main_layout.add_widget(buttons_layout)
    

        return main_layout


    def set_time(self, instance):
        # Clear the message label initially
        self.message_label.text = ''
        try:
            # Convert input text to integer
            user_input = int(self.timing_input_min.text)*60+int(self.timing_input_sec)
            global cmd_queue
            self.message_label.text = f"Setting capture time to {user_input} minutes in process..."
            pkt = Cmdpacket(Cmd.SETINTERVAL, user_input)
            if not cmd_queue.full():
                cmd_queue.put(pkt)
        except ValueError:
            # Handle the case where the input is not an integer
            self.message_label.text = "Invalid input. Please enter a whole number."

    # Callback function to handle the 'Create Video' button press
    def create_video(self, instance):
        self.message_label.text = f"creating video now..."
        # Get today's date in the desired format (YYYY-MM-DD)
        today_date = datetime.now().strftime('%Y-%m-%d')
        filename = today_date + '.m4v'
        def task():
            command = f'ffmpeg -framerate 2 -pattern_type glob -i \"img/*.jpg\" -c:v libx264 -r 30 {filename} -y'
            result = subprocess.run(command, shell=True, capture_output=True)
            
            if result.returncode == 0:
                self.message_label.text = f"Done. Saved as {filename}"
            else:
                self.message_label.text = f"Error: {result.stderr.decode()}"


        Thread(target=task).start()

    def power_off(self, instance):
        global cmd_queue
        self.message_label.text = f"Set to poweroff."
        pkt = Cmdpacket(Cmd.POWEROFF, Cmd.POWEROFF)
        if not cmd_queue.full():
            cmd_queue.put(pkt) 
        


if __name__ == '__main__':
    app = PictureCaptureApp()
    app.run()
