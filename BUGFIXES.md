# Bug Fixes - FTXUI Implementation

## Overview
This document summarizes the critical bugs that were fixed in the FTXUI todo application.

## Issues Fixed

### 1. Two Description Fields Displayed
**Problem:** When opening the "Add Todo" dialog, two description input fields were visible instead of one.

**Root Cause:** Incorrect usage of the `window()` function in FTXUI v5. The code was using:
```cpp
// WRONG - window() was chained at the end
return vbox(content) | border | center | window(text("Title"), filler());
```

**Solution:** The `window()` function must wrap the content, not be chained as a decorator:
```cpp
// CORRECT - window() wraps the content
return window(text("Title"), vbox(content) | border) | center;
```

**Files Changed:**
- `src/app.cpp` - All dialog rendering functions

### 2. Input Field Not Editable
**Problem:** Users could not type in the description field when adding/editing todos.

**Root Cause:** The broken window() rendering was preventing proper focus and event handling. The input handling code was correct (using `event.is_character()`), but the rendering prevented it from working.

**Solution:** Fixed the window() usage allowed the input event handling to work properly.

**Files Changed:**
- `src/app.cpp` - Dialog event handlers

### 3. ESC Key Not Working
**Problem:** Pressing ESC did not cancel dialogs and return to main view.

**Root Cause:** Dialog rendering was broken, preventing proper event propagation.

**Solution:** Fixed window() usage and ensured ESC handling occurs first in the event chain:
```cpp
// ESC always cancels dialogs
if (event == Event::Escape && m_dialogState != DialogState::None) {
    m_dialogState = DialogState::None;
    return true;
}
```

**Files Changed:**
- `src/app.cpp` - Event handler

### 4. Dialog Taking Full Screen
**Problem:** Dialogs were not properly centered and took up the entire screen instead of appearing as modal boxes.

**Root Cause:** Incorrect window() usage prevented proper dialog sizing and centering.

**Solution:** Proper window() wrapper allows FTXUI to render centered modal dialogs with correct borders.

**Files Changed:**
- `src/app.cpp` - Dialog rendering functions

## Technical Details

### FTXUI window() Function Usage

**Incorrect Pattern (Before):**
```cpp
return vbox(content) | border | center | window(text("Title"), filler());
```
This creates type mismatches and rendering issues because `window()` expects specific arguments.

**Correct Pattern (After):**
```cpp
return window(text("Title"), vbox(content) | border) | center;
```
This properly wraps the content in a window with a title, then applies centering.

### Text Input Handling

The application uses manual character capture for text input (instead of the Input component which had focus issues):

```cpp
if (event.is_character()) {
    m_dialogDescription += event.character();
    return true;
}
if (event == Event::Backspace) {
    if (!m_dialogDescription.empty()) {
        m_dialogDescription.pop_back();
    }
    return true;
}
```

## Branch Information

**Fixed Branch:** `fix-dialog-rendering`

To use the fixed version:
```bash
git checkout fix-dialog-rendering
cd build
cmake ..
make
./todo_tui
```

## Testing Checklist

After applying fixes, verify:
- [ ] Press `n` to add todo - single centered dialog appears
- [ ] Type in description field - text appears correctly
- [ ] Press `ESC` - dialog cancels and returns to main view
- [ ] Press `1`, `2`, or `3` - priority changes correctly
- [ ] Press `Enter` - todo saves correctly
- [ ] All other dialogs (Edit, Delete, Search, Help) work similarly
- [ ] No duplicate UI elements appear

## Related Commits

- Initial FTXUI implementation: `5ac44baafc07d2ff3b33ad93f638204686650e60`
- Window() fix commit: See `fix-dialog-rendering` branch

---

**Note:** The application is already using FTXUI (not ncurses). These fixes address rendering bugs in the FTXUI implementation.