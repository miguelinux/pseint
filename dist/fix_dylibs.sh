cd $1/Contents || exit 1

LIST="MacOS/pseint Resources/psterm Resources/pseval Resources/psdrawE Resources/psdraw3"
mkdir -p Frameworks

WXDIR=$(whereis wx-config | cut -d ':' -f 2 | sed 's/\/bin\/wx-config//')

while ! [ "$LIST" = "" ]; do
	NEXT_LIST=""
	for FILE in $LIST; do

		echo Processing $FILE...
		for FLIB in $(x86_64-apple-darwin18-otool -L $FILE | grep -v : | grep libwx | cut -d '(' -f 1); do 
            NLIB=$(echo $FLIB|rev|cut -d '/' -f 1|rev)
			echo "    found $NLIB"
            x86_64-apple-darwin18-install_name_tool -change $FLIB @executable_path/../Frameworks/$NLIB $FILE
            if ! [ -f Frameworks/$NLIB ]; then 
				cp $WXDIR/lib/$NLIB Frameworks/ 
				NEXT_LIST="$NEXT_LIST Frameworks/$NLIB"
			fi
		done
# 		for FLIB in $(x86_64-apple-darwin18-otool -L $FILE | grep /opt/mac | cut -d '(' -f 1); do 
# 			NLIB=$(echo $FLIB|rev|cut -d '/' -f 1|rev)
# 			echo "    found $NLIB"
# 			x86_64-apple-darwin18-install_name_tool -change $FLIB @executable_path/../Frameworks/$NLIB $FILE
# 			if ! [ -f Frameworks/$NLIB ]; then 
# 				cp $FLIB Frameworks/$NLIB; 
# 				NEXT_LIST="$NEXT_LIST Frameworks/$NLIB"
# 			fi
# 		done
	
	done
	LIST="$NEXT_LIST"
done
