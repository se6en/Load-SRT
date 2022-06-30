# Load-SRT
# 6/24/2022
Implement the function of parse format information from subtitle file. Including bold(<>), italic, underline and color indformation.
Color tag may be hex value string, color name string or color rgb value string, also support to convert them to C++ format color value COLORREF.
These format will be applied while drawing the content.

Known issue: Only support file encoded with UTF8 yet.

# 4/22/2021
Load SubRip Subtitle File (.srt) without format information.

## Sample project files
---
### SRTDataManager(.h and .cpp)
---
Here are there kinds of information of each subtitle: start time, end time, and content. Store these information in the vector.

### DlgLoadSRTProgress(.h and .cpp)
Load subtitle information with a thread, and show progress in this window. After the thread complete, it will post a message to the window and shut it.

In the *.srt file, here are some format.
1. Each one should contain serial number, start time --> end time and content. All of them are indispensable.
2. One finished with a space line. The serial number just stand for the order. Checked in Premiere Pro, the file can be load successfully even thought the serial number is discontinuous. 
3. If any of these three elements was missed, all the subtitle information after this one (include itself) will not be include. But the subtitle before this one will be loaded with no error information.(Checked Premiere Pro) 

### StaticDrawSRT(.h and .cpp)
Draw subtitle with Direct2D. 

### Load SRTDlg(.h and .cpp)
A button was used for select subtitle file. Progress control will update the subtitle with progress.
