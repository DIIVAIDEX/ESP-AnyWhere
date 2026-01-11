idf_component_register(
    SRCS
         "WG.c"
          "Src/crypto.c"
         "Src/wireguard.c"
         "Src/wireguardif.c"
         "Src/wireguard-platform.c"
         "Src/crypto/refc/blake2s.c"
         "Src/crypto/refc/chacha20.c"
         "Src/crypto/refc/chacha20poly1305.c"
         "Src/crypto/refc/poly1305-donna.c"
         "Src/crypto/refc/x25519.c"
         "Src/esp_wireguard.c"
         "Src/nacl/crypto_scalarmult/curve25519/ref/smult.c"
    INCLUDE_DIRS "Inc"
    PRIV_INCLUDE_DIRS "Src"
    REQUIRES esp_netif lwip mbedtls SyncTime)

if(${IDF_VERSION_MAJOR} STREQUAL 5)
    set_source_files_properties(Src/crypto/refc/x25519.c
        PROPERTIES COMPILE_FLAGS
        -Wno-error=stringop-overread)
endif()
