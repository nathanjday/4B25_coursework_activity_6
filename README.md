# 4B25_coursework_activity_6
Nathan Day, Trinity College, nd368.

SUMMARY:

This folder contains the software pertaining to the pedometer developed with the FRDMKL03Z board. A clear description of files included in this folder is given below.

FOREWORD ON CODE STRUCTURE:

All code has been written and structured in a manner readily understood by someone familiar with the systems involved. Comments have been added where appropriate.

CONTAINED FOLDERS:

- srec_files

Contains srec files corresponding to the source code files mentioned below.

CONTAINED FILES:
- README.md

- 4B25_Final_Project_Report.pdf

- test_program_accelerometer_CSV.c

Source code for test program in which data is received from registers 1-6 of the accelerometer unit (the registers corresponding to accelerometer X, Y, Z readings), and outputted to the terminal (via serial) in CSV format.

- test_program_accelerometer_CSV_data_raw.csv

CSV file containing the data outputted from accelerometer test program.

- test_program_accelerometer_CSV_data_analysis.csv

Brief data reconstruction and analysis attempts.

- test_program_OLED_numbers.c

Source code for test program in which OLED displays any 4 digit number, as it would do when functioning in the pedometer.

- test_program_OLED_numbers_image.jpg

Image showing OLED output when OLED test program running.

- main.c

Source code for the program designed for integration of system components, which would be the program deployed on the packaged device. Note that this is not fully complete due to time constraints from problems encountered earlier in project development.


NOTE OF THANKS:

Thank you to the lecturer for providing a modern, interesting, and mostly enjoyable course.

Slightly less thank you to the manufacturer of the FRDM-KL03Z board.
