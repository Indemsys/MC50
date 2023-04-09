import os
import string
import xml.etree.ElementTree as ET

prog_file_name = "MC50.ewp"
proj_dir = os.getcwd() # Файл должен находится в корневой директории проекта

#------------------------------------------------------------------------------------------------------------
# Функция рекурсивного прохода вглубь по директориям с корнем заданным в аргументе root_path и создание субблоков в дереве xml_tree 
#------------------------------------------------------------------------------------------------------------
def Create_IAR_groups_tree(current_path, xml_tree):
  n = 0

  # Создаем группу
  iar_group = ET.SubElement(xml_tree, 'group') # Создаем XML тэг group в дереве переданном в качестве аргумента xml_tree - это будет блок содержащий элементы каталога в дереве IAR
  name_tag = ET.SubElement(iar_group, 'name')  # В тэге group создаем тэг name - это будет именем группы в дереве каталогов IAR
  name_tag.text = os.path.basename(current_path)  # Получаем полный путь к текущей директории

  # Рекурсивно проходим по дереву директорий 
  dirs_iterator = os.scandir(current_path)
  for directory_object in dirs_iterator:
    if (not (directory_object.name.startswith('.') 
       or directory_object.name.startswith('Debug')) 
       and directory_object.is_dir()):
      deeper_path = os.path.join(current_path,directory_object.name)
      print ('Group: ' + deeper_path)
      n = n + Create_IAR_groups_tree(deeper_path, iar_group)  #Рекурсивно уходим в глубь дерева директорий

  # Здесь 
  dirs_iterator = os.scandir(current_path)
  for directory_object in dirs_iterator:
    if not directory_object.name.startswith('.') and directory_object.is_file():
      fname = os.path.join(current_path,directory_object.name)
      if (fname.endswith('.c')
      or fname.endswith('.cpp')
      or fname.endswith('.s')
      or fname.endswith('.S')
      or fname.endswith('.h')
      or fname.endswith('.hpp')
      or fname.endswith('.a')):
        p = fname.replace(proj_dir,"$PROJ_DIR$")
        file_tag = ET.SubElement(iar_group, 'file')   # Вставлем тэг файл в текущее дерево группы
        name_tag = ET.SubElement(file_tag,  'name')   # Вставляем в блок тэга file блок name с именем файла  
        name_tag.text = p
        n = n + 1  
        print ('File:  ' + p)


  if n==0:
    xml_tree.remove(iar_group) # Если не было записано ни одного файла, то уже вставленый каталог удаляем  

  ET.indent(iar_group, space=" ", level=0)
  return n

#============================================================================================================
# Начало выполнения программы
#============================================================================================================

full_proj_name = proj_dir + "\\" + prog_file_name  # Получаем полный путь к файлу проекта IAR

# Открываем файл проекта

tree = ET.parse(full_proj_name) # Парсим XML файл проекта IAR

xml_root = tree.getroot() 
  
xml_configuration_block = xml_root.findall('configuration') # Находим блок с конфигурацией 

#............................................................................................................
# Итерация по всем найденным блокам конфигураций

for configuration_object  in xml_configuration_block:

  for settings_object in configuration_object.iter('settings'):
    name_object = settings_object.find('name')

    # Находим блок содержащий все опции текущей конфигурации <configuration>...<settings>...<name>ICCARM</name>..<data> ....  </data>...</settings>... </configuration>

    if name_object.text == 'ICCARM':
      ICCARM_data_object = settings_object.find('data')
      break

  # Преобразуем список подключаемых путей так чтобы в него попали все поддиректории корня проекта

  # Ищем и удаляем блок CCIncludePath2 с перечислением подключаемых путей
  for option in ICCARM_data_object.iter('option'):
    opt_name = option.find('name')
    if opt_name.text == 'CCIncludePath2':
      ICCARM_data_object.remove(option)
      break


  # Восстанавливаем блок option с перечислением путей чтобы заново в нем записать пути
  new_opt = ET.SubElement(ICCARM_data_object, 'option')
  new_name = ET.SubElement(new_opt, 'name')
  new_name.text = 'CCIncludePath2'



  # Проходим по всем директориям проекта и включаем их в список
  # Если включать только директории содержащие .h файлы то при компиляции проекта IAR IDE не находит некоторых директорй после такого преобразования
  for dirpath, dirs, files in os.walk(proj_dir):
    if not (dirpath.startswith(proj_dir+"\\Out")
            or dirpath.startswith(proj_dir+"\\.")
            or dirpath.startswith(proj_dir+"\\ParametersGenerator")
            or dirpath.startswith(proj_dir+"\\debug")
            or dirpath.startswith(proj_dir+"\\Debug")
            or dirpath.startswith(proj_dir+"\\release")
            or dirpath.startswith(proj_dir+"\\.hg")
            or dirpath.startswith(proj_dir+"\\SE_proj")
            or dirpath.startswith(proj_dir+"\\.settings")
            or dirpath.startswith(proj_dir+"\\settings")
            or dirpath.startswith(proj_dir+"\\FreeMaster")
            or dirpath.startswith(proj_dir+"\\ParametersManager")
            or dirpath.startswith(proj_dir+"\\script")
            or dirpath.startswith(proj_dir+"\\Traces")
            or dirpath.startswith(proj_dir+"\\.tracing")
            or dirpath.startswith(proj_dir+"\\Image_packer")
            or (dirpath == proj_dir)):
      p = dirpath.replace(proj_dir, "$PROJ_DIR$")
      new_incl = ET.SubElement(new_opt, 'state')
      new_incl.text = p
      print('Include: ' + p)

  ET.indent(new_opt, space=" ", level=0)

  
# Конец итерации по конфигурациям for configuration_object  in xml_configuration_block:
#............................................................................................................
    
# Удаляем список груп и файлов в группах и формируем новый с распределением по группам аналогичным распределению по директориям
# Список файлов и груп в проект один для всех конфигураций
groups = xml_root.findall('group')
for group  in groups:
  #ET.dump(group)
  xml_root.remove(group)
  
  
Create_IAR_groups_tree(proj_dir , xml_root)  
  
    

print ("END!")
tree.write(full_proj_name, method="xml", xml_declaration=True, short_empty_elements=False, encoding="UTF-8")


