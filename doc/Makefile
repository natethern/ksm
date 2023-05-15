all: ksm.info.gz

ksm.info.gz: ksm.texi
	rm -rf ksm.info*
	makeinfo ksm.texi
	gzip ksm.info*

clean:
	rm -f *.aux *.cp *.cps *.dvi *.fn *.fns *.info* *.ky *.log \
              *.pg *.toc *.tp *.vr *~ 

html: ksm.texi
	rm -f ksm*.html
	texi2html -split_chapter ksm.texi

install: 
	install -D ksm.1 $(mandir)/man1/ksm.1
	install -D ksm.info.gz $(infodir)/ksm.info.gz
	install ksm.info-*.gz $(infodir)
	@if $(INSTALL_INFO) --version 2>&1 | sed 1q | fgrep -i debian \
            >/dev/null ;\
          then \
            if [ ! -f $(infodir)/dir ]; then install dir $(infodir); fi ;\
            $(INSTALL_INFO) --info-dir=$(infodir) \
                            --section 'Programming' Programming \
                            $(infodir)/ksm.info.gz ;\
          else \
            $(INSTALL_INFO) $(infodir)/ksm.info.gz $(infodir)/dir ; \
          fi

uninstall:
	rm -f $(mandir)/man1/ksm.1
	rm -f $(infodir)/ksm.info*
	$(INSTALL_INFO) --remove $(infodir)/ksm.info.gz $(infodir)/dir



