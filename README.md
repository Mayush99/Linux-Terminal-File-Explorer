# Linux-Terminal-File-Explorer
<br/>
Objective:<br/>
Build a fully functional File Explorer Application, with a restricted feature set.<br/>
Prerequisites:<br/>
1. Basic usage and architectural know-how of file explorer features<br/>
2. Preliminaries such as C/C++ code compilation, execution & debugging.<br/>
3. <br/>
<b>Requirements:</b><br/>
Your File Explorer should work in two modes :-<br/>
1. Normal mode (default mode) - used to explore the current directory and navigate the filesystem<br/>
2. Command mode - used to enter shell commands<br/>
The root of your application should be the same as the system, and the home of the application should
be the same as home of the current user.<br/>
The application should display data starting from the top-left corner of the terminal window, line-by-
line. You should be able to handle text rendering if the terminal window is resized. The last line of the
display screen is to be used as a status bar.<br/>
<br/>
<b>Normal mode:</b><br/>
Normal mode is the default mode of your application. It should have the following functionalities-<br/>
1. Display a list of directories and files in the current folder<br/>
a. Every file in the directory should be displayed on a new line with the following
attributes for each file -<br/>
i.File Name<br/>
ii.File Size<br/>
iii.Ownership (user and group) and Permissions<br/>
iv.Last modified<br/>
<br/>
b. The file explorer should show entries “.” and “..” for current and parent directory
respectively<br/>
c. The file explorer should handle scrolling using the up and down arrow keys.<br/>
d. User should be able to navigate up and down in the file list using the corresponding up<br/>
and down arrow keys. The up and down arrow keys should also handle scrolling during
vertical overflow.<br/>
<br/>
2. Open directories and files When enter key is pressed -<br/>
a.
Directory - Clear the screen and navigate into the directory and show
the directory contents as specified in point 1<br/>
b.
File - Open the file in vi editor<br/>
<br/>
3. Traversal<br/>
a. Go back - Left arrow key should take the user to the previously visited directory<br/>
b. Go forward - Right arrow key should take the user to the next directory<br/>
c. Up one level - Backspace key should take the user up one level<br/>
d. Home – h key should take the user to the home folder<br/>
<br/>
<b>Command Mode:</b><br/>
The application should enter the Command button whenever “:” (colon) key is pressed. In the command
mode, the user should be able to enter different commands. All commands appear in the status bar at the
bottom.<br/>
1. Copy –<br/>
‘$ copy <source_file(s)> <destination_directory>’<br/>
Move –<br/>
‘$ move <source_file(s)> <destination_directory>’<br/>
Rename –<br/>
‘$ rename <old_filename> <new_filename>’a. <br/>
Eg –
‘$ copy foo.txt bar.txt baz.mp4 ~/foobar’<br/>
‘$ move foo.txt bar.txt baz.mp4 ~/foobar’<br/>
‘$ rename foo.txt bar.txt’<br/>
b. Assume that the destination directory exists, and you have write permissions.<br/>
c. Copying/Moving directories should also be implemented<br/>
d. The file ownership and permissions should remain intact<br/><br/>
2. Create File –<br/>
‘$ create_file <file_name> <destination_path>’<br/><br/>
Create Directory –<br/>
‘$ create_dir <dir_name> <destination_path>’<br/>
a. Eg – ‘$ create_file foo.txt ~/foobar create_file foo.txt’.<br/>
‘$ create_dir foo ~/foobar’<br/><br/>
3. Delete File –<br/>
‘$ delete_file <file_path>’<br/><br/>
Delete Directory –<br/>
‘$ delete_dir <dir_path>’<br/>
a. On deleting directory, you must recursively delete all content present inside it
.<br/><br/>
4. Goto –</br>
‘$ goto <location>’<br/>
a.
  Eg – ‘$ goto <directory_path>’</br>
</br>5. Search –</br>
‘$ search <file_name>’
or
  ‘$ search <directory_name>’</br>
  a. Search for a given file or folder under the current directory recursively.</br>
b. Output should be True or False depending on whether the file or folder exists6. On pressing ESC key, the application should go back to Normal Mode</br>
</br>7. On pressing q key in normal mode, the application should close. Similarly, entering the ‘quit’
command in command mode should also close the application.</br>
