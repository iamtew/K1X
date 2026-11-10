// PixiJS backdrop: catalog kickScene, full-viewport, no CDN.
const C = { k1xOrange: 0xd94214, k1xSteel: 0xb8c4d6 };
const BPM = 124;
const BEAT = 60 / BPM;
const reduceMotion = matchMedia('(prefers-reduced-motion: reduce)').matches;
const rand = (lo, hi) => lo + Math.random() * (hi - lo);

// ?fx / ?fx=true keeps only the canvas. ?fx=false / 0 / off stays a normal page.
const fxRaw = new URLSearchParams(location.search).get('fx');
const fxOnly = fxRaw !== null && !/^(0|false|off)$/i.test(fxRaw);
if (fxOnly) document.body.classList.add('fx-only');

const pointer = { x: 0.5, y: 0.5 };
if (!fxOnly) {
  addEventListener('pointermove', (e) => {
    pointer.x = e.clientX / innerWidth;
    pointer.y = e.clientY / innerHeight;
  }, { passive: true });
}

start().catch((err) => {
  console.info('fx: animated backdrop disabled:', err?.message || err);
  document.getElementById('fx')?.remove();
});

async function start() {
  const PIXI = await import('./vendor/pixi.min.mjs');
  const app = new PIXI.Application();
  await app.init({
    canvas: document.getElementById('fx'),
    resizeTo: window,
    backgroundAlpha: 0,
    antialias: true,
    autoDensity: true,
    resolution: Math.min(devicePixelRatio || 1, 2),
    preference: 'webgl',
  });

  const glow = glowTexture(PIXI);
  const scene = kickScene(PIXI, glow);
  app.stage.addChild(scene.root);
  bootTicker(app, scene);
}

function bootTicker(app, scene) {
  if (reduceMotion) {
    app.ticker.stop();
    const still = () => {
      const { width: w, height: h } = app.screen;
      scene.update(8, w, h, 0);
      app.render();
    };
    addEventListener('resize', () => requestAnimationFrame(still));
    still();
    return;
  }

  let t = 0;
  app.ticker.add((ticker) => {
    const dt = Math.min(ticker.deltaMS / 1000, 0.05);
    t += dt;
    const { width: w, height: h } = app.screen;
    // Preview mode is full-bleed, so there is no page to parallax against.
    if (!fxOnly) {
      const ease = Math.min(1, dt * 2.5);
      scene.root.x += ((0.5 - pointer.x) * 40 - scene.root.x) * ease;
      scene.root.y += ((0.5 - pointer.y) * 24 - scene.root.y) * ease;
    }
    scene.update(t, w, h, dt);
  });

  document.addEventListener('visibilitychange', () => {
    if (document.hidden) app.ticker.stop(); else app.ticker.start();
  });
}

function glowTexture(PIXI, size = 64) {
  const c = document.createElement('canvas');
  c.width = c.height = size;
  const g = c.getContext('2d');
  const r = size / 2;
  const grd = g.createRadialGradient(r, r, 0, r, r, r);
  grd.addColorStop(0, 'rgba(255,255,255,1)');
  grd.addColorStop(0.25, 'rgba(255,255,255,0.5)');
  grd.addColorStop(1, 'rgba(255,255,255,0)');
  g.fillStyle = grd;
  g.fillRect(0, 0, size, size);
  return PIXI.Texture.from(c);
}

function glowSprite(PIXI, tex, tint) {
  const s = new PIXI.Sprite(tex);
  s.anchor.set(0.5);
  s.tint = tint;
  s.blendMode = 'add';
  return s;
}

const GLOW_PASSES = [[22, 0.05], [10, 0.14], [2.5, 1]];
function glowLine(g, pts, color, scale = 1, passes = GLOW_PASSES) {
  for (const [width, alpha] of passes) {
    g.moveTo(pts[0], pts[1]);
    for (let i = 2; i < pts.length; i += 2) g.lineTo(pts[i], pts[i + 1]);
    g.stroke({ width: width * scale, color, alpha, cap: 'round', join: 'round' });
  }
}

function kickScene(PIXI, glow) {
  const root = new PIXI.Container();
  const rings = new PIXI.Graphics();
  const wave = new PIXI.Graphics();
  rings.blendMode = 'add';
  wave.blendMode = 'add';
  const core = glowSprite(PIXI, glow, C.k1xOrange);
  root.addChild(rings, wave, core);

  const sparks = Array.from({ length: 90 }, () => {
    const s = glowSprite(PIXI, glow, Math.random() < 0.7 ? C.k1xOrange : C.k1xSteel);
    s.alpha = 0;
    root.addChild(s);
    return s;
  });
  let nextSpark = 0;
  const bursts = [];
  let lastBeat = -1;

  return {
    root,
    update(t, w, h, dt) {
      const cx = w * 0.5, cy = h * 0.45;
      const beat = Math.floor(t / BEAT);
      const env = Math.exp(-(t % BEAT) * 7);
      if (beat !== lastBeat) {
        lastBeat = beat;
        bursts.push(t);
        for (let i = 0; i < 24; i++) {
          const s = sparks[nextSpark++ % sparks.length];
          const a = rand(0, Math.PI * 2), v = rand(200, 650);
          s.x = cx; s.y = cy; s.vx = Math.cos(a) * v; s.vy = Math.sin(a) * v;
          s.alpha = 1; s.scale.set(rand(0.15, 0.4));
        }
      }
      while (bursts.length && t - bursts[0] > 2.6) bursts.shift();

      rings.clear();
      const maxR = Math.hypot(w, h) * 0.55;
      for (const born of bursts) {
        const a = t - born, life = 1 - a / 2.6;
        if (life <= 0) continue;
        const r = maxR * (1 - Math.exp(-a * 2.2));
        for (const [width, alpha] of GLOW_PASSES) {
          rings.circle(cx, cy, r).stroke({ width: width * (0.5 + life), color: C.k1xOrange, alpha: alpha * life * life });
        }
        if (a > 0.12) {
          rings.circle(cx, cy, r * 0.82).stroke({ width: 1.5, color: C.k1xSteel, alpha: 0.5 * life * life });
        }
      }

      core.scale.set(2.5 + 5 * env);
      core.alpha = 0.35 + 0.6 * env;

      for (const s of sparks) {
        if (s.alpha <= 0.01) continue;
        s.x += s.vx * dt; s.y += s.vy * dt;
        s.vx *= Math.exp(-dt * 2.5); s.vy *= Math.exp(-dt * 2.5);
        s.alpha *= Math.exp(-dt * 2.2);
      }

      wave.clear();
      const pts = [], wy = h * 0.82, amp = h * 0.09 * (0.45 + 0.55 * env);
      for (let x = 0; x <= w; x += 3) {
        const u = x / w;
        const phase = 70 * u + 55 * (1 - Math.exp(-u * 6)) - t * 6;
        pts.push(x, wy + amp * Math.exp(-u * 2.8) * Math.sin(phase));
      }
      glowLine(wave, pts, C.k1xOrange);
    },
  };
}
