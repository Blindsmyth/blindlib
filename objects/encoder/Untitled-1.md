
I want to save resources. that is why I want to fuse these dial objects into one big object.
configurations are going to be stored in a table.



Static positioning:
8 dials arranged in 2 rows (4 each)
First row: y=13, Second row: y=39
x positions: 26, 52, 78, 104 (for each row)
width=24, height=24 for all dials
Per-page configuration:
Modes: 0=frac unipolar, 1=frac bipolar, 2=int, 3=list
Labels
Show/hide value
List options for selector mode
Display structures:
Uses dial_t for frac modes
Uses intdisplay_t for int mode
Uses select_t for list mode
Input:
Initializes all display structures at startup
