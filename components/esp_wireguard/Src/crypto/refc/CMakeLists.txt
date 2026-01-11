set(srcs "blake2s.c"
	     "chacha20.c"
	     "chacha20poly1305.c"
	     "poly1305-donna.c"
	     "x25519.c")
idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ".")
