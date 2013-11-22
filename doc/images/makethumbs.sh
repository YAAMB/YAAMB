#!/bin/bash
# Ordner "thumbs" anlegen
mkdir -p thumbs
gm mogrify -size 300x300 -output-directory thumbs *.png *.jpg
