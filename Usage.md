# Software Installation for TomTom Go x30 #

To install, download a binary `.zip` file and uncompress it to the internal storage of your device. **Make sure that you have at least one "computer voice" installed.** (The folder `LoquendoTTS` must be present.)

After you disconnect your device, an icon with the caption "Lecturer" should appear on the menu.

# Installing Texts #

Copy texts you want to read to the directory `Lecturer/text`. Lecturer supports both UTF-8 and ISO-8859-1 encodings. It does not, however, support non-Latin scripts at the moment. For best results, it is highly recommended to use an unformatted text file, meaning one that only contains newline characters on paragraph breaks.

# Main screen #

When you start Lecturer for the first time, it will take you to the file dialog where you can pick a text file to read by touching its name. If you have used Lecturer before, it will continue with the file and at the position at which you quit the last time.

Page through your file by touching the left or right half of the screen to go back or forward, respectively.

You can quit Lecturer by touching the "Quit" button in the top left corner. (Note that the buttons are very small in order not to consume too much screen real estate. Their sensitive area is bigger to make the interface usable without a pen.)

To pick a new file, press the "File" button. It will take you to the file dialog where you can pick a new file to read. Lecturer will remember the position within the current file and will return back to it when you open that file again. If you change your mind and want to continue reading the current file, press the "Back" button.

On the bottom of the screen you see the progress bar. It visualizes the current position within the file viewed. You can touch the progress bar to quickly jump to another page. Note that the sensitive area of the progress bar is much smaller than for the buttons to keep you from accidentally touching it.

You can see the current page's number on the bottom right of the screen.

On the bottom left of the screen, there is the "Talk" button. Touch it, and Lecturer will read the text out loud, starting from the top of the current page. When it reaches the end of the page, it will automatically turn to the next one and continue reading. Touch the "Talk" button again and it will stop reading.

# Options #

In the top center of the main screen you will find the "Options" button, which will take you to the options screens. You can choose between the "Rendering", "About", and "Speech" tabs by touching the buttons at the bottom of the options screen. You can save the current settings as default for new files by touching "Save as default", and you can reset the options to the default by touching "Reset". The "Back" button will take you back to the main screen.

## Rendering ##

This tab allows you to change the margin and spacing sizes used for displaying text. Each value has a plus and minus button you can use to change it. The unit for margins is pixels, the unit for spacing is "percent of current font height". The current margins setting is demonstrated by the box drawn around the dialog.

You can also change the font size here. The current setting is illustrated by the small "A" next to the font size buttons.

The progress bar, page numbering, and justification can be turned on or off here as well.

## About ##

The about tab tells you about author, license, and the release number of the Lecturer build you are running.

## Speech ##

This tab allows you to pick a voice used for speech output, if you have more than one installed. Language is implicit, so if you pick a French voice, for instance, it will only be able to read French text correctly.