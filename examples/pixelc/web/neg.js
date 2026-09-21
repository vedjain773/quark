let instance;

async function loadWasm() {
  const response = await fetch("../obj/output.wasm");
  const bytes = await response.arrayBuffer();

  const result = await WebAssembly.instantiate(bytes, {});
  instance = result.instance;
}

loadWasm();

const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");
const upload = document.getElementById("upload");

upload.onchange = () => {
  const file = upload.files[0];
  const img = new Image();

  img.src = URL.createObjectURL(file);

  img.onload = () => {
    canvas.width = img.width;
    canvas.height = img.height;

    ctx.drawImage(img, 0, 0);

    // processImage();

    const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
    const bench = createBenchmark(instance, img.width, img.height, 21);

    bench(imageData);
  };
};

function processImage() {
  const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
  const data = imageData.data;

  const ptr = instance.exports.__heap_base.value;
  console.log("__heap_base:", ptr);

  const needed_bytes = ptr + data.length;
  if (instance.exports.memory.buffer.byteLength < needed_bytes) {
    const needed_pages = Math.ceil(
      (needed_bytes - instance.exports.memory.buffer.byteLength) / 65536,
    );
    instance.exports.memory.grow(needed_pages);
  }

  const mem = new Uint8Array(instance.exports.memory.buffer);
  mem.set(data, ptr);

  instance.exports.neg(ptr, data.length);

  const result = new Uint8Array(
    instance.exports.memory.buffer,
    ptr,
    data.length,
  );
  data.set(result);
  ctx.putImageData(imageData, 0, 0);
}

function negJS(data) {
  let i = 0;
  while (i + 4 <= data.length) {
    data[i] = 255 - data[i];
    data[i + 1] = 255 - data[i + 1];
    data[i + 2] = 255 - data[i + 2];
    
    i += 4;
  }
}

function createBenchmark(instance, width, height, runs = 21) {
  const length = width * height * 4;

  const ptr = instance.exports.__heap_base.value;

  const requiredBytes = ptr + length;
  const currentBytes = instance.exports.memory.buffer.byteLength;

  if (currentBytes < requiredBytes) {
    const pagesNeeded = Math.ceil((requiredBytes - currentBytes) / 65536);
    instance.exports.memory.grow(pagesNeeded);
  }

  let wasmView = new Uint8ClampedArray(
    instance.exports.memory.buffer,
    ptr,
    length,
  );

  const jsBuffer = new Uint8ClampedArray(length);

  return function runBenchmark(imageData) {
    wasmView.set(imageData.data);
    jsBuffer.set(imageData.data);

    for (let i = 0; i < 5; i++) {
      instance.exports.neg(ptr, length);
      negJS(jsBuffer);
    }

    wasmView.set(imageData.data);
    jsBuffer.set(imageData.data);

    let wasmTotal = 0;
    for (let i = 0; i < runs; i++) {
      console.log(`Iteration: ${i}`);
      const t1 = performance.now();
      instance.exports.neg(ptr, length);
      const t2 = performance.now();
      wasmTotal += t2 - t1;
    }

    let jsTotal = 0;
    for (let i = 0; i < runs; i++) {
      const t1 = performance.now();
      negJS(jsBuffer);
      const t2 = performance.now();
      jsTotal += t2 - t1;
    }

    const wasmAvg = wasmTotal / runs;
    const jsAvg = jsTotal / runs;

    console.log(`WASM avg: ${wasmAvg.toFixed(2)} ms`);
    console.log(`JS   avg: ${jsAvg.toFixed(2)} ms`);
    console.log(`Speed ratio (JS / WASM): ${(jsAvg / wasmAvg).toFixed(2)}x`);

    const output = new ImageData(wasmView, width, height);
    ctx.putImageData(output, 0, 0);

    return { wasmAvg, jsAvg };
  };
}
