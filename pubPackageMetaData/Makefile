include /Developer/Makefiles/pb_makefiles/platform.make


SRCDIRS = .
DESTDIR = $(DSTROOT)/System/Installation/

OBJROOT=.
SYMROOT=.

installhdrs::

clean::
	find . \( -name '#*' -o -name '.places' -o -name '.*.wmd' -o -name '.macros' -o -name '.nfs*' -o -name '.list' -o -name '.DS_Store' -o -name '._*' -o -name '._.*' \) -exec rm -rf {} \;


installsrc:: $(SRCROOT)
	cp Makefile $(SRCROOT) 
	@for dir in $(SRCDIRS); \
	do \
    		echo == $$dir == ;\
 	   	ditto -V $$dir $(SRCROOT)/$$dir;\
	done

install:: $(DESTDIR)
	@for dir in $(SRCDIRS) ;\
	do \
	    	echo == $$dir == ;\
	    	ditto -V $$dir $(DESTDIR)/$$dir;\
	done 
	rm -f $(DESTDIR)Makefile
	rm -f $(DESTDIR)releasenotes.txt
	echo "== Setting permissions in $(DESTDIR) ==  "
	chown -R root.wheel $(DESTDIR) 
	chmod -R 755 $(DESTDIR) 
	(find $(DESTDIR) \( -type f  \) \( -name '*.rtf' -o -name '*.txt' -o -name '*.info' -o -name '*.list' -o -name '*.bomout' -o -name 'PkgInfo' -o -name '*.plist' -o -name '*.tiff' -o -name '*.nib' -o -name '*.strings' \) -a -exec chmod 644 {}  \;)
	echo "== Finished setting permissions in $(DESTDIR) == "


${DESTDIR} ${SRCROOT} ${DSTROOT}:; mkdir -p -m 755 $@
