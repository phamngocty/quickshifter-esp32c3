# -*- coding: utf-8 -*-
import os, re

with open('docs/index.html', 'r', encoding='utf-8') as f:
    content = f.read()

print(f"Total characters: {len(content)}")
print(f"Total lines: {len(content.splitlines())}")

# 1. Check for forbidden hardware/chip keywords
forbidden = ['esp32', 'esp-32', 'risc-v', 'littlefs', 'supermini', 'platformio', 'gpio']
found_forbidden = []
for word in forbidden:
    matches = re.findall(rf'\b{word}\b', content, re.IGNORECASE)
    if matches:
        found_forbidden.append((word, len(matches)))
print('Forbidden keywords check:', found_forbidden if found_forbidden else 'PASSED (0 leaks)')

# 2. Check all image references
images = re.findall(r'(?:src|href|openLightbox\()[\'"]([^\'"]*images/[^\'"]*)[\'"]', content)
unique_images = sorted(list(set(images)))
print(f'Unique image references ({len(unique_images)}):', unique_images)
missing_images = []
for img in unique_images:
    local_path = os.path.join('docs', img.replace('/', os.sep))
    if not os.path.exists(local_path):
        missing_images.append((img, local_path))
print('Missing images:', missing_images if missing_images else 'PASSED (All exist)')

# 3. Check Nav Links vs Section IDs
nav_links = re.findall(r'<a\s+href="#([^"]+)"', content)
section_ids = set(re.findall(r'id="([^"]+)"', content))
print('Nav links target:', nav_links)
print('Section IDs:', sorted(list(section_ids)))
missing_targets = [link for link in nav_links if link not in section_ids]
print('Missing link targets:', missing_targets if missing_targets else 'PASSED (All target IDs exist)')

# 4. Check for broken encoding characters like \ufffd
broken_chars = content.count('\ufffd')
print('Broken utf-8 replacement characters count:', broken_chars)

# 5. Check JS syntax / script tags
scripts = re.findall(r'<script>(.*?)</script>', content, re.DOTALL)
print(f'Found {len(scripts)} inline script block(s).')

# 6. Basic HTML tag balance check
tags = ['<html', '</html>', '<head', '</head>', '<body', '</body>', '<header', '</header>', '<nav', '</nav>', '<footer', '</footer>']
for t in tags:
    print(f'{t}: {content.count(t)}')
