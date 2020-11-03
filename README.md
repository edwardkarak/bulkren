# bulkren
Utility to rename files in a directory in bulk. Useful for giving numbered names to files, ex: file1.txt, file2.txt, file3.txt, ... 
Subdirectories are not renamed.

## Usage
First click the "Choose Directory..." button. A dialog will appear where you can select the directory whose files you want to rename. Select the directory and click OK.

In the following textbox, enter the pattern to be used in renaming the files. A series of consecutive (at least one) pound signs indicates a possibly zero-padded number should replace it. The minimum number of digits is equal to the number of pound signs.  For example ## means a two-digit number should replace the pound symbols. If the number has fewer than two digits, the left digit is a zero. If the number has greater than 2 digits, then the number will be formatted with no padding. A series of consecutive pound signs may not appear more than once in a pattern.

## Example:

Pattern: `img###.png`

Output:
```
img001.png
img002.png
...
img999.png
img1000.png
...
```

## Options

* "Target files only of the given extension" will cause only the files whose extension matches that given in the pattern to be renamed. If no extension is given in the pattern and this option is selected, then files having no extension will be targeted.

* "Auto-calculate no. of digits of padding" will cause the program to calculate automatically the number of digits of padding needed based on the total number of files that are to be renamed. For instance, if 350 files need to be renamed, the number of padding digits will be set to 3. Therefore, the number of pound signs indicated in the pattern is ignored.

* The radio buttons in the "Radix" group box allow the user to select the radix, or base, using which the number will be formatted. "Decimal" is the default. For "Hexadecimal" and "Octal" modes, the number will be prefixed with `0x` or `0o`, respectively.
