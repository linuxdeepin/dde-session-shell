#!/bin/bash
lupdate ./ -ts -no-obsolete translations/dde-session-shell_en_US.ts
#tx push -s -b m20 # Push the files that need to be translated
#tx pull -s -b m20 # Pull the translated files
