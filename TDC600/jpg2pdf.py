#!/usr/bin/python
from fpdf import FPDF
from PIL import Image
import os, re
pdf = FPDF('P', 'mm', (217,256))
exts = ['jpg']
files = [f for f in os.listdir('.') if any(f.endswith(ext) for ext in exts)]
for image in sorted(files):
	pdf.add_page()
	pdf.image(image,0,0,217)
pdf.output("TDC600.pdf", "F")
