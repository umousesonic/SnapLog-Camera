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
import subprocess
from datetime import datetime
from share_queue import shared_queue


class PictureCaptureApp(App):
    def build(self):
        # Overall vertical box
        main_layout = BoxLayout(orientation='vertical')

        # Label for displaying validation messages
        self.message_label = Label(size_hint_y=None, height=30)
        main_layout.add_widget(self.message_label)

        # First box: Label for setting time
        setting_label = Label(text='Setting time for auto capturing...', size_hint_y = 0.5, font_size = 64)
        main_layout.add_widget(setting_label)

        # Second box: Centered horizontal box with TextInput and 'min' Label
        timing_layout = BoxLayout(orientation='horizontal', size_hint_y=None, height=50, spacing=10)
        # Use a dummy widget with a flexible size_hint_x to push your widgets to the center
        dummy_left = Widget(size_hint_x=1)
        self.timing_input = TextInput(multiline=False, size_hint_x=None, width=200)
        timing_label = Label(text='min', size_hint_x=None, width=50)
        dummy_right = Widget(size_hint_x=1)
        # Add the dummy widgets and the input + label
        timing_layout.add_widget(dummy_left)
        timing_layout.add_widget(self.timing_input)
        timing_layout.add_widget(timing_label)
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
            user_input = int(self.timing_input.text)
            # Check if the input is in the correct range
            if 1 <= user_input <= 5:
                global shared_queue
                self.message_label.text = f"Setting capture time to {user_input} minutes in process..."
                if not shared_queue.full():
                    shared_queue.put(user_input)
            else:
                self.message_label.text = "Invalid input. Please enter a number between 1 and 5."
        except ValueError:
            # Handle the case where the input is not an integer
            self.message_label.text = "Invalid input. Please enter a whole number."

    # Callback function to handle the 'Create Video' button press
    def create_video(self, instance):
        self.message_label.text = ''
        self.message_label.text = f"creating video now..."
        # Get today's date in the desired format (YYYY-MM-DD)
        today_date = datetime.now().strftime('%Y-%m-%d')

        # Specify the base directory where the command should be run
        # base_folder_path = '/Users/brianhuang/Desktop'
        base_folder_path = '/Users/brianhuang/Desktop/image'

        # The specific folder path for today's date
        today_folder_path = os.path.join(base_folder_path, today_date)

        # The command you want to run (without the username and host information)
        command = 'ffmpeg -framerate 1 -i image%d.png -c:v libx264 -r 30 output.m4v'

        # Split the command into parts to pass it to subprocess
        command_list = command.split()

        # Execute the command
        # subprocess.run(command_list, cwd=today_folder_path)
        subprocess.run(command_list, cwd=base_folder_path)

