import argparse
from enum import Enum
import json
import shutil
import subprocess
import logging
from dataclasses import dataclass
from pathlib import Path


OUTPUT_SUFFIX = '.dds'
FORMAT = 'BC7_UNORM'


logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.DEBUG)


def get(json_dict, *keys):
    acc = json_dict
    for key in keys:
        acc = acc.get(key)
        if acc is None:
            return
    return acc


class TextureType(Enum):
    ALBEDO = 0
    NORMAL = 1
    PBR = 2
    EMISSIVE = 3


@dataclass
class TextureConversionCommand:
    texture_idx: int
    file_path: Path
    texture_type: TextureType = TextureType.ALBEDO


@dataclass
class EditCommand:
    image_idx: int
    new_uri: str


class GltfTextureConverter:
    def __init__(self, gltf_file_path, output_path, texconv_path):
        self.gltf_path = Path(gltf_file_path)
        self.output_path = Path(output_path)
        self.texconv = Path(texconv_path)
        assert self.gltf_path.exists()
        assert self.texconv.exists()

        with open(self.gltf_path, 'r') as f:
            self.gltf_json = json.load(f)

        self.conversion_commands: list[TextureConversionCommand] = []
        self.edit_commands: list[EditCommand] = []

    def process(self):
        self.output_path.mkdir(exist_ok=True)
        self._generate_commands()
        self._process_gltf_edit_commands()
        self._process_texture_commands()
        binary_files = self.gltf_path.parent.glob('**/*.bin')
        for bin_file in binary_files:
            shutil.copy2(bin_file, dst=self.output_path)

    def _generate_commands(self):
        materials = self.gltf_json['materials']
        for i, material in enumerate(materials):
            pbrProperties = material.get('pbrMetallicRoughness')

            if pbrProperties is None:
                logger.warning(f'Material with index {i} does not have PBR properties')
                continue

            texture_idx = get(pbrProperties, 'baseColorTexture', 'index')
            command = TextureConversionCommand(
                texture_idx,
                self._get_texture_path(texture_idx),
                TextureType.ALBEDO
            )
            self._add_command(command)

            texture_idx = get(pbrProperties, 'metallicRoughnessTexture', 'index')
            command = TextureConversionCommand(
                texture_idx,
                self._get_texture_path(texture_idx),
                TextureType.PBR
            )
            self._add_command(command)

            texture_idx = get(material, 'normalTexture', 'index')
            command = TextureConversionCommand(
                texture_idx,
                self._get_texture_path(texture_idx),
                TextureType.NORMAL
            )
            self._add_command(command)

            texture_idx = get(material, 'occlusionTexture', 'index')
            command = TextureConversionCommand(
                texture_idx,
                self._get_texture_path(texture_idx),
                TextureType.PBR
            )
            self._add_command(command)

            texture_idx = get(material, 'emissiveTexture', 'index')
            command = TextureConversionCommand(
                texture_idx,
                self._get_texture_path(texture_idx),
                TextureType.EMISSIVE
            )
            self._add_command(command)

    def _get_texture_path(self, texture_idx):
        if texture_idx is None:
            return

        texture_dict = self.gltf_json['textures'][texture_idx]
        img_uri = self.gltf_json['images'][texture_dict['source']]['uri']
        return self.gltf_path.parent / img_uri

    def _add_command(self, command: TextureConversionCommand):
        if command.texture_idx is None:
            return

        texture_dict = self.gltf_json['textures'][command.texture_idx]
        img_uri = self.gltf_json['images'][texture_dict['source']]['uri']

        self.conversion_commands.append(command)
        self.edit_commands.append(EditCommand(
            image_idx=texture_dict['source'],
            new_uri=str(Path(img_uri).with_suffix(OUTPUT_SUFFIX)),
        ))

    def _process_texture_commands(self):

        for command in self.conversion_commands:
            input_file = str(command.file_path)
            logger.info('Parsing: ' + input_file)
            args = [str(self.texconv), input_file, '-o', str(self.output_path), '-ft', 'DDS']

            format = FORMAT

            if command.texture_type is TextureType.ALBEDO or command.texture_type is TextureType.EMISSIVE:
                args.append('-srgbi')
                format = f'{format}_SRGB'

            args.append('-f')
            args.append(format)

            logger.info('Using arguments: ' + ' '.join(args))
            subprocess.run(args, check=True)

    def _process_gltf_edit_commands(self):
        images = self.gltf_json['images']
        for idx, command in enumerate(self.edit_commands):
            images[command.image_idx]['uri'] = command.new_uri

        new_gltf_path = self.output_path.joinpath(self.gltf_path.name)
        with open(new_gltf_path, 'x') as f:
            json.dump(self.gltf_json, f)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'gltf_path',
        help='The path to the folder containing texture assets you wish to convert',
    )

    parser.add_argument(
        'output_path',
        help='The path where you would like to place the modified files',
    )
    parser.add_argument(
        '-t',
        '--texconv_path',
        help='The path to the texturec.exe executable tool',
        default='./tools/texconv.exe',
    )
    args = parser.parse_args()
    converter = GltfTextureConverter(args.gltf_path, args.output_path, args.texconv_path)
    converter.process()
