# Roadmap de engenharia

## P0 — fundação
- [x] Android NativeActivity em C++20
- [x] EGL/OpenGL ES 3
- [x] Entrada de toque
- [x] Pressão/inclinação NDK quando reportadas
- [x] Undo/redo simples de traços
- [x] Ferramenta borracha rudimentar e paleta rápida

## P1 — desenho utilizável
- [ ] Mesh de pincel com bordas suaves e espaçamento adaptativo
- [ ] Borracha real com camada/FBO ou operações não destrutivas
- [ ] Palm rejection, stylus-only e multitouch pan/zoom
- [ ] Camadas, opacidade, blend modes
- [ ] Seletor de cor e editor de pincéis
- [ ] Salvar/carregar projeto com versão de formato

## P2 — animação
- [ ] Flipbook e onion skin
- [ ] Timeline multi-track e edição de clips
- [ ] Keyframes e curvas de easing
- [ ] Playback determinístico e áudio sincronizado

## P3 — Performing e mídia
- [ ] Captura de transformações em tempo real
- [ ] Edição de passes gravados
- [ ] Importação de imagens, vídeo e áudio
- [ ] Exportação via MediaCodec/MediaMuxer

## P4 — qualidade de produção
- [ ] Testes unitários e instrumentados
- [ ] Testes em GPUs Adreno/Mali
- [ ] Limites de memória e recuperação de falhas
- [ ] Benchmark de latência de input e frame time
- [ ] Assinatura e pipeline de release

Cada item só deve ser marcado como concluído depois de implementação e teste reproduzível.
