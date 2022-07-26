# Load-SRT
# 7/26/2022
Update the method of extracting format infomation from the caption files. Now we will split the content into lines first, then we just extcat the infomation line by line. This is the behavior I decided to make after I checked Premiere Pro. I also add a way to export caption as .txt file. Only content will be exported without the time information.

# 6/30/2022
Implement loading SRT files encoded with ANSI, UTF-8, UTF-8-BOM, UTF-16 BE BOM and UTF-16 LE BOM. Currently the application will preload the file to distinguish the encoder of the file at first. Then it will use <i>std::wbuffer_convert</i> to covert the file stream and read each line in the file.

According to my test, all these files can be loaded correctly. Both content and format are kept as expected.

But since I found that the <i>std::wbuffer_convert</i> is deprecated since C++17, I create another branch to try to read the content without extra interface. Will be merged into the main branch after I finished.

# 6/24/2022
Implement the function of parse format information from subtitle file. Including <b>bold</b>, <i>italic</i>, <u>underline</u> and <font color=red>color</font> indformation.
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
