@echo off
cargo build
cd regression
python regression.py
cd ..