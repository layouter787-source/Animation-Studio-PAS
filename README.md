# Animation Studio PAS

Editor de animação 2D para Android, independente, com motor nativo C++20, Android NDK, CMake e OpenGL ES 3.

## Estado real — versão 0.2.0

Esta entrega adiciona uma primeira tela nativa interativa:
- canvas OpenGL ES 3;
- desenho por toque;
- leitura de pressão e inclinação disponibilizadas pelo dispositivo;
- desfazer/refazer por botões no topo;
- alternância simples de borracha;
- duas cores rápidas.

**Limitações conhecidas desta versão:** o renderizador atual usa pontos GPU para representar os traços, a borracha é uma aproximação visual e ainda não é uma borracha por composição alfa correta; não há persistência de projeto, camadas, pincéis avançados, interface de timeline, animação por frames, áudio/vídeo, Performing completo ou exportação. O suporte de pressão/inclinação depende do hardware e do digitalizador. O projeto ainda precisa ser compilado e testado em dispositivos Android reais.

## Compilar

1. Abrir a pasta no Android Studio.
2. Instalar Android SDK 35, Android NDK e CMake 3.22.1.
3. Sincronizar Gradle e compilar o módulo `app`.
4. Testar primeiro num dispositivo arm64 com Android 8.0 ou superior.

## Estrutura

- `app/src/main/cpp/main.cpp`: NativeActivity, ciclo EGL, entrada e renderização.
- `app/src/main/cpp/CMakeLists.txt`: build nativo.
- Gradle/Manifest: empacotamento Android.

## Roteiro técnico

1. Motor de pincéis com malhas trianguladas, interpolação de pressão e borracha alfa.
2. Camadas, seleção, undo/redo transacional e gravação de projetos.
3. Timeline, flipbook, onion skin, keyframes, interpolação e playback.
4. Performing com captura de gestos e edição de passes.
5. Importação e exportação de imagem, áudio e vídeo via APIs Android.
6. Testes automatizados, perfil de GPU/memória, otimização e APK assinado.

Não é uma cópia do código, dos recursos ou da marca Procreate. A compatibilidade será construída por implementação independente.
