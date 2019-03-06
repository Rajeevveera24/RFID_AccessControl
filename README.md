# RFID_AccessControl
This project is aimed at setting up an access control mechanism using RFID scanners in the workshops area of my college.

I have used an arduino nano, along with an RFID reader and an SD card to implement this project.
A student can gain access to the workshop of the team that he/she is a part of using their card (which has a unique UID).
The program will scan their UID and if it is present in the database (Which is stored on an SD card), they will be granted access to the club that they are a part of (which will be displayed on a 16X2 LCD screen)

New UIDs can be added to database of the resoective club using a master card (which belongs to the head of the club/team).

The mechanism works like this: To add a new card to the database, first scan the master card (of the respective club), and then scan the card to be added. To remove an already existing card, do the exact same procedure.



PS:
The LCD screen I worked with isn't working at present, so details are being displayed to the serial monitor. The changes will be implemented soon.

The current implementation has a few drawbacks, and the current focus is to device a better mecahnism to delete an already existing card.
I plan to incorporate a node MCU, along with the nano to make the process more efficient.

Disclaimer: I have referred to the following link for this project (https://electronicshobbyists.com/rfid-based-access-control-system-using-arduino/)

While the idea is similar, the code is original
