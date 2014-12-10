Who worked on which parts of the project: For the most part we both worked on imagefill.cpp, Flint implemented a few more functions.  Nicholas worked on makematte.cpp.

Finished parts of the project:
	-infilling
	-outpainting
	-matte making

How to use the code:
	-use make to compile the main program (imagefill) and "make makematte" to make the matte drawing program
	-for infilling run:
		./imagefill [in file] [matte] [out file] [patch radius]
	-for outpainting run:
		./imagefill [in file] [out file] [width] [height] [x] [y] [patch radius]
	where x and y refer to the coordinate of the top left corner of the original image in the new image

What we learned:
	-exhaustively searching all candidate patches for the best matching patch and calculating the SSD for each patch may be unnecessarily inefficient
	-the patch size makes a difference on how well the algorithm performs and the patch size should be different for different images

Results are available in the pdf in this folder.