COMPONENT_SRCDIRS := src/
COMPONENT_ADD_INCLUDEDIRS := include/
COMPONENT_EMBED_FILES := web.bin

WEB_SOURCES = $(COMPONENT_PATH)/package.json $(wildcard $(COMPONENT_PATH)/src/*.ts) $(wildcard $(COMPONENT_PATH)/src/*.vue)

#libweb.o: web.bin
$(COMPONENT_PATH)/include/web-server.h: $(COMPONENT_PATH)/web.bin
	cd $(COMPONENT_PATH) && node web.js

# $(COMPONENT_PATH)/web.bin: $(WEB_SOURCES)
# 	cd $(COMPONENT_PATH) && npm run build
