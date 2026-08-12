// Configuration
const TARGET_SIZE = { width: 320, height: 240 };
const PREVIEW_CELL_SIZE = { width: 64, height: 48 };
const PREVIEW_GRID = { width: 5, height: 5 };
const IMAGES_PER_PREVIEW = PREVIEW_GRID.width * PREVIEW_GRID.height; // 25

// State
let images = [];
let resizeMode = 'crop'; // 'scale', 'crop' or 'padding'
let quality = 80;
let generatedNwip = null;
let outputFilename = 'output.nwip';

// DOM elements
const dropZone = document.getElementById('dropZone');
const fileInput = document.getElementById('fileInput');
const imagesList = document.getElementById('imagesList');
const imageCount = document.getElementById('imageCount');
const qualityInput = document.getElementById('quality');
const qualityValue = document.getElementById('qualityValue');
const modeToggle = document.getElementById('modeToggle');
const generateBtn = document.getElementById('generateBtn');
const clearBtn = document.getElementById('clearBtn');
const terminal = document.getElementById('terminal');
const statusDiv = document.getElementById('status');
const downloadBtn = document.getElementById('downloadBtn');
const fileInfo = document.getElementById('fileInfo');
const previewCanvas = document.getElementById('previewCanvas');
const previewFrame = document.querySelector('.preview-frame');
const previewLabel = document.getElementById('previewLabel');

// Magick instance (not needed for basic JPEG conversion - using Canvas native API)
let Magick = null;

// Initialize
async function initMagick() {
    syncQualityControl();
    addLog('Image processor initialized (using Canvas API)', 'success');
}

function syncQualityControl() {
    // Force default to 80 on load/refresh to avoid browser form-preserve behavior
    quality = 80;
    qualityInput.value = '80';
    qualityValue.textContent = '80';
}

function updateQualityFromPointer(event) {
    const rect = qualityInput.getBoundingClientRect();
    const min = Number(qualityInput.min);
    const max = Number(qualityInput.max);
    const step = Number(qualityInput.step || 1);
    const ratio = Math.min(1, Math.max(0, (event.clientX - rect.left) / rect.width));
    const rawValue = min + ratio * (max - min);
    const snappedValue = Math.round(rawValue / step) * step;
    const nextValue = Math.min(max, Math.max(min, snappedValue));
    const oldQuality = quality;
    qualityInput.value = String(nextValue);
    quality = nextValue;
    qualityValue.textContent = String(quality);
    return nextValue !== oldQuality;
}

// Logger
function addLog(message, type = 'info') {
    if (terminal.querySelector('.empty-state')) {
        terminal.innerHTML = '';
    }
    
    const entry = document.createElement('div');
    entry.className = `log-entry ${type}`;
    entry.textContent = `[${new Date().toLocaleTimeString()}] ${message}`;
    terminal.appendChild(entry);

    while (terminal.children.length > 50) {
        terminal.removeChild(terminal.firstChild);
    }

    terminal.scrollTop = terminal.scrollHeight;
}

dropZone.addEventListener('click', () => {
    fileInput.click();
});

fileInput.addEventListener('change', (e) => {
    handleFiles(e.target.files);
});

// Handle file selection
function handleFiles(files) {
    const supportedExtensions = {'.jpg': true, '.jpeg': true, '.png': true, '.webp': true};
    
    for (let file of files) {
        const ext = '.' + file.name.split('.').pop().toLowerCase();
        if (supportedExtensions[ext]) {
            const reader = new FileReader();
            reader.onload = async (e) => {
                const blob = new Blob([e.target.result], { type: file.type });
                const blobUrl = URL.createObjectURL(blob);
                
                images.push({
                    name: file.name,
                    size: file.size,
                    data: e.target.result,
                    blobUrl: blobUrl
                });
                addLog(`Added: ${file.name} (${formatFileSize(file.size)})`, 'info');
                updateImagesList();
                updateGenerateButton();
                await updatePreview();
            };
            reader.readAsArrayBuffer(file);
        } else {
            addLog(`Unsupported: ${file.name}`, 'warning');
        }
    }
}

// Update images list UI
function updateImagesList() {
    imageCount.textContent = images.length;
    
    if (images.length === 0) {
        imagesList.innerHTML = '<div class="empty-state">No images added yet</div>';
        return;
    }
    
    imagesList.innerHTML = images.map((img, idx) => `
        <div class="image-item">
            <img src="${img.blobUrl}" class="image-thumbnail" alt="${img.name}">
            <div class="image-info">
                <div class="image-name">${img.name}</div>
                <div class="image-size">${formatFileSize(img.size)}</div>
            </div>
            <button class="remove-btn" onclick="removeImage(${idx})">Remove</button>
        </div>
    `).join('');
}

// Remove image
function removeImage(idx) {
    if (images[idx].blobUrl) {
        URL.revokeObjectURL(images[idx].blobUrl);
    }
    images.splice(idx, 1);
    addLog(`Removed image ${idx + 1}`, 'info');
    updateImagesList();
    updateGenerateButton();
    updatePreview();
}

// Update generate button state
function updateGenerateButton() {
    generateBtn.disabled = images.length === 0;
}

// Format file size
function formatFileSize(bytes) {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
}

// Update preview for first image
async function updatePreview() {
    const displayWidth = previewFrame.clientWidth;
    const displayHeight = previewFrame.clientHeight;
    previewCanvas.width = displayWidth;
    previewCanvas.height = displayHeight;
    const ctx = previewCanvas.getContext('2d');
    ctx.clearRect(0, 0, previewCanvas.width, previewCanvas.height);

    if (images.length === 0) {
        previewLabel.textContent = 'No preview available';
        return;
    }

    try {
        const canvas = await processImage(images[0]);
        const previewImage = new Image();
        previewImage.src = canvas.toDataURL('image/jpeg', quality / 100);

        await new Promise((resolve, reject) => {
            previewImage.onload = resolve;
            previewImage.onerror = reject;
        });

        ctx.clearRect(0, 0, previewCanvas.width, previewCanvas.height);
        ctx.drawImage(previewImage, 0, 0, previewCanvas.width, previewCanvas.height);
        previewLabel.textContent = `${images[0].name} — ${resizeMode.charAt(0).toUpperCase() + resizeMode.slice(1)} @${quality}`;
        addLog(`Preview updated for ${images[0].name}`, 'info');
    } catch (error) {
        previewLabel.textContent = 'Preview generation failed';
        addLog(`Preview error: ${error.message}`, 'error');
    }
}

// Quality slider
qualityInput.addEventListener('input', async (e) => {
    quality = Number(e.target.value);
    qualityValue.textContent = String(quality);
    // Disable download if settings changed
    if (generatedNwip) {
        generatedNwip = null;
        downloadBtn.disabled = true;
        fileInfo.innerHTML = 'Settings changed - regenerate to download';
        statusDiv.className = 'status';
        statusDiv.style.display = 'none';
        addLog('Settings changed - please regenerate', 'warning');
    }
    await updatePreview();
});

// Pointer interactions: click anywhere on the track and drag
qualityInput.addEventListener('pointerdown', async (e) => {
    e.preventDefault();
    const changed = updateQualityFromPointer(e);
    if (changed) {
        if (generatedNwip) {
            generatedNwip = null;
            downloadBtn.disabled = true;
            fileInfo.innerHTML = 'Settings changed - regenerate to download';
            statusDiv.className = 'status';
            statusDiv.style.display = 'none';
            addLog('Settings changed - please regenerate', 'warning');
        }
        await updatePreview();
    }

    const onMove = async (ev) => {
        const changed = updateQualityFromPointer(ev);
        if (changed) await updatePreview();
    };

    const onUp = () => {
        window.removeEventListener('pointermove', onMove);
        window.removeEventListener('pointerup', onUp);
    };

    window.addEventListener('pointermove', onMove);
    window.addEventListener('pointerup', onUp);
});

// Mode toggle
modeToggle.addEventListener('click', async (e) => {
    if (e.target.classList.contains('toggle-option')) {
        document.querySelectorAll('#modeToggle .toggle-option').forEach(opt => {
            opt.classList.remove('active');
        });
        e.target.classList.add('active');
        resizeMode = e.target.dataset.mode;
        addLog(`Resize mode changed to: ${resizeMode}`, 'info');
        // Disable download if settings changed
        if (generatedNwip) {
            generatedNwip = null;
            downloadBtn.disabled = true;
            fileInfo.innerHTML = 'Settings changed - regenerate to download';
            statusDiv.className = 'status';
            statusDiv.style.display = 'none';
            addLog('Settings changed - please regenerate', 'warning');
        }
        await updatePreview();
    }
});

// Clear all
clearBtn.addEventListener('click', () => {
    // Revoke blob URLs
    for (let img of images) {
        if (img.blobUrl) {
            URL.revokeObjectURL(img.blobUrl);
        }
    }
    images = [];
    generatedNwip = null;
    quality = 80;
    syncQualityControl();
    addLog('Cleared all images', 'info');
    updateImagesList();
    updateGenerateButton();
    downloadBtn.disabled = true;
    fileInfo.innerHTML = 'No file generated yet';
    statusDiv.className = 'status';
    previewLabel.textContent = 'No preview available';
    const clearWidth = previewFrame.clientWidth;
    const clearHeight = previewFrame.clientHeight;
    previewCanvas.width = clearWidth;
    previewCanvas.height = clearHeight;
    const ctx = previewCanvas.getContext('2d');
    ctx.clearRect(0, 0, previewCanvas.width, previewCanvas.height);
    terminal.innerHTML = '<div class="empty-state">Waiting for input...</div>';
});

// Resize/crop image
async function processImage(imageData) {
    try {
        const img = new Image();
        img.src = imageData.blobUrl;
        
        await new Promise((resolve, reject) => {
            img.onload = resolve;
            img.onerror = reject;
        });

        // Convert to canvas
        const canvas = document.createElement('canvas');
        const ctx = canvas.getContext('2d');
        
        if (resizeMode === 'scale') {
            canvas.width = TARGET_SIZE.width;
            canvas.height = TARGET_SIZE.height;
            ctx.drawImage(img, 0, 0, TARGET_SIZE.width, TARGET_SIZE.height);
        } else if (resizeMode === 'padding') {
            const targetRatio = TARGET_SIZE.width / TARGET_SIZE.height;
            const imgRatio = img.width / img.height;

            canvas.width = TARGET_SIZE.width;
            canvas.height = TARGET_SIZE.height;
            ctx.fillStyle = '#000000';
            ctx.fillRect(0, 0, TARGET_SIZE.width, TARGET_SIZE.height);

            let drawWidth, drawHeight, drawX, drawY;
            if (imgRatio > targetRatio) {
                drawWidth = TARGET_SIZE.width;
                drawHeight = TARGET_SIZE.width / imgRatio;
            } else {
                drawHeight = TARGET_SIZE.height;
                drawWidth = TARGET_SIZE.height * imgRatio;
            }

            drawX = (TARGET_SIZE.width - drawWidth) / 2;
            drawY = (TARGET_SIZE.height - drawHeight) / 2;
            ctx.drawImage(img, drawX, drawY, drawWidth, drawHeight);
        } else {
            const ratio = Math.max(TARGET_SIZE.width / img.width, TARGET_SIZE.height / img.height);
            const newWidth = img.width * ratio;
            const newHeight = img.height * ratio;
            const offsetX = (newWidth - TARGET_SIZE.width) / 2;
            const offsetY = (newHeight - TARGET_SIZE.height) / 2;

            canvas.width = TARGET_SIZE.width;
            canvas.height = TARGET_SIZE.height;
            ctx.drawImage(img, -offsetX, -offsetY, newWidth, newHeight);
        }

        return canvas;
    } catch (error) {
        addLog(`Error processing image: ${error.message}`, 'error');
        throw error;
    }
}

// Convert canvas to JPEG bytes
function canvasToJpeg(canvas, jpegQuality) {
    return new Promise((resolve) => {
        canvas.toBlob((blob) => {
            const reader = new FileReader();
            reader.onload = () => {
                resolve(new Uint8Array(reader.result));
            };
            reader.readAsArrayBuffer(blob);
        }, 'image/jpeg', jpegQuality / 100);
    });
}

// Build preview image
async function buildPreview(canvases) {
    const previewCanvas = document.createElement('canvas');
    previewCanvas.width = TARGET_SIZE.width;
    previewCanvas.height = TARGET_SIZE.height;
    const ctx = previewCanvas.getContext('2d');
    ctx.fillStyle = '#000000';
    ctx.fillRect(0, 0, TARGET_SIZE.width, TARGET_SIZE.height);

    const cellWidth = PREVIEW_CELL_SIZE.width;
    const cellHeight = PREVIEW_CELL_SIZE.height;

    for (let i = 0; i < IMAGES_PER_PREVIEW; i++) {
        const x = (i % PREVIEW_GRID.width) * cellWidth;
        const y = Math.floor(i / PREVIEW_GRID.width) * cellHeight;

        if (i < canvases.length) {
            const img = new Image();
            img.src = canvases[i].toDataURL();
            
            await new Promise(resolve => {
                img.onload = () => {
                    const tmpCanvas = document.createElement('canvas');
                    tmpCanvas.width = cellWidth;
                    tmpCanvas.height = cellHeight;
                    const tmpCtx = tmpCanvas.getContext('2d');
                    tmpCtx.drawImage(img, 0, 0, cellWidth, cellHeight);
                    ctx.drawImage(tmpCanvas, x, y);
                    resolve();
                };
            });
        }
    }

    return previewCanvas;
}

// Encode to binary format
function encodeToBinary(jpegDataList, previewCount) {
    const totalImages = jpegDataList.length;
    const headerSize = 8 + 8 * totalImages;
    
    // Calculate offsets and sizes
    const offsets = [];
    const sizes = [];
    let currentOffset = headerSize;
    
    for (let data of jpegDataList) {
        offsets.push(currentOffset);
        sizes.push(data.length);
        currentOffset += data.length;
    }

    // Create binary buffer
    const buffer = new ArrayBuffer(currentOffset);
    const view = new DataView(buffer);
    let offset = 0;

    // Write header: nb_previews and nb_total_images
    view.setUint32(offset, previewCount, true);
    offset += 4;
    view.setUint32(offset, totalImages, true);
    offset += 4;

    // Write offset/size pairs
    for (let i = 0; i < totalImages; i++) {
        view.setUint32(offset, offsets[i], true);
        offset += 4;
        view.setUint32(offset, sizes[i], true);
        offset += 4;
    }

    // Write image data
    const uint8View = new Uint8Array(buffer);
    for (let data of jpegDataList) {
        uint8View.set(data, offset);
        offset += data.length;
    }

    return uint8View;
}

function getOutputFilename() {
    return `output_i${images.length}_q${quality}_${resizeMode}.nwip`;
}

// Main generation function
generateBtn.addEventListener('click', async () => {
    if (images.length === 0) return;

    generateBtn.disabled = true;
    statusDiv.style.display = 'block';
    statusDiv.className = 'status processing';
    statusDiv.textContent = 'Processing…';
    terminal.innerHTML = '';
    addLog(`Starting image processing (Quality: ${quality}, Mode: ${resizeMode})`, 'info');

    try {
        const processedCanvases = [];
        const jpegDataList = [];

        // Process each image
        for (let i = 0; i < images.length; i++) {
            try {
                addLog(`[${i + 1}/${images.length}] Processing: ${images[i].name}`, 'info');

                const canvas = await processImage(images[i]);
                processedCanvases.push(canvas);

                const jpegData = await canvasToJpeg(canvas, quality);
                jpegDataList.push(jpegData);

                addLog(`  ✓ Converted to JPEG (${formatFileSize(jpegData.length)})`, 'success');
            } catch (error) {
                addLog(`  ✗ Error: ${error.message}`, 'error');
            }
        }

        if (jpegDataList.length === 0) {
            throw new Error('No images were processed successfully');
        }

        // Generate previews
        addLog(`Generating preview images...`, 'info');
        const previewCount = Math.ceil(processedCanvases.length / IMAGES_PER_PREVIEW);
        const previewJpegList = [];

        for (let i = 0; i < previewCount; i++) {
            const start = i * IMAGES_PER_PREVIEW;
            const end = Math.min(start + IMAGES_PER_PREVIEW, processedCanvases.length);
            const previewCanvases = processedCanvases.slice(start, end);

            const previewCanvas = await buildPreview(previewCanvases);
            const previewJpeg = await canvasToJpeg(previewCanvas, quality);
            previewJpegList.push(previewJpeg);

            addLog(`  Preview ${i + 1}/${previewCount}: ${formatFileSize(previewJpeg.length)}`, 'success');
        }

        // Combine: previews first, then individual images
        const finalJpegList = [...previewJpegList, ...jpegDataList];

        // Encode to binary
        addLog('Encoding to binary format...', 'info');
        generatedNwip = encodeToBinary(finalJpegList, previewCount);
        outputFilename = getOutputFilename();

        const totalSize = generatedNwip.byteLength;
        addLog(`✓ Generated ${outputFilename} (${formatFileSize(totalSize)})`, 'success');
        addLog(`  - Previews: ${previewCount}`, 'info');
        addLog(`  - Total images: ${jpegDataList.length}`, 'info');

        // Update UI
        statusDiv.className = 'status success';
        statusDiv.textContent = '✓ Generation complete!';

        downloadBtn.disabled = false;
        fileInfo.innerHTML = `<strong>File:</strong> ${outputFilename}<br><strong>Size:</strong> ${formatFileSize(totalSize)}<br><strong>Images:</strong> ${jpegDataList.length} + ${previewCount} preview(s)`;

    } catch (error) {
        addLog(`✗ Generation failed: ${error.message}`, 'error');
        statusDiv.className = 'status error';
        statusDiv.textContent = '✗ Generation failed';
    } finally {
        generateBtn.disabled = false;
    }
});

// Download button
downloadBtn.addEventListener('click', () => {
    if (!generatedNwip) return;

    const blob = new Blob([generatedNwip], { type: 'application/octet-stream' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    const filename = getOutputFilename();
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);

    addLog(`✓ Downloaded: ${filename}`, 'success');
});

// Initialize
initMagick();