# coding=utf8
# the above tag defines encoding for this document and is for Python 2.x compatibility

import re
import os
def get_index_of_line_start(lines):

    pre_line = lines[:-1]
    index = 0 
    for line in pre_line:
        index += len(line)
    return index

def build_function_template(func_name,params,return_type):
    template = '\n/**\n * @brief '+ func_name + '\n * \n'

    for param in params:
        template += ' * @param ' + param + '\n'

    template += ' * @return Please Place Description here. \n'
    template += ' * @retval ' + return_type + '\n'

    template += ' */\n'

    return template

def function_add_comment(file_path):
    if(file_path.startswith('./os')):
            category = 'OS'
    else:
        category = ''
        pass
    os.system('ctags -x  --sort=no --c-types=p ' + file_path + ' > ./tmp')
    dump_info  = open('./tmp', 'r') 
    dump_info  = dump_info.read()
    src_file = open(file_path, 'r') 
    file_lines = src_file.readlines()
    src_file.seek(0)
    file_chars = src_file.read()
    src_file.close()
    
    regex = r"(\w*)[ ]*prototype[ ]*([0-9]+) (.*\.h) (.*)"

    matches = re.findall(regex, dump_info, re.MULTILINE)
    matches = sorted(matches, key = lambda function_info: int(function_info[1]))
    # print(matches)
    count = 0
    append_line_count = 0
    for match in matches:
        lineno = int(match[1]) + append_line_count
        func_name = (match[0])
        index = get_index_of_line_start(file_lines[:lineno])
        # print(match[0])
        # print('----------',file_chars[index :index +100].index(match[0]),'++++++++++++')
        arg_start = file_chars.find('(',index)
        arg_end   = file_chars.find(')',index)
        # return_type_end     = file_chars.rfind(' ',0,index + file_chars[index :index +100].index(match[0])) 
        excluded_pattern = ['\n',' ']
        return_type_end = file_chars.rfind(' ',0,index + file_chars[index :index +100].index(match[0])) 
            
        while True:
            if(file_chars[return_type_end] in  excluded_pattern):
                return_type_end -= 1
            else:
                break
            
        return_type_start = return_type_end - 1
        while True:
            if(file_chars[return_type_start] not in  excluded_pattern):
                return_type_start -= 1
            else:
                break
            
            
        # return_type_start_space      = file_chars.rfind(' ',0,return_type_end)
        # return_type_start_nxt_line   = file_chars.rfind('\n',0,return_type_end)
        
        # return_type_start = return_type_start_space+1 if(return_type_start_space > return_type_start_nxt_line)  else return_type_start_nxt_line
        
        return_type = file_chars[return_type_start:return_type_end+1].replace('\n','')
        
        
        if(count == 0):
            define_start = file_chars.rfind('\n',0,index) - 1 
        else:
            define_start = file_chars.rfind(';',0,index) 
            
        # print('-----',file_chars[define_start+1:index],file_chars[index:arg_start],file_chars[arg_start:arg_end+1],'+++++++++++')
        
        
        args = file_chars[arg_start+1:arg_end].split(',')
        params = list()
        for arg in args :
            arg_type = ' '.join(arg.split()[:-1])
            arg_name = arg.split()[-1]
            # print('type:',arg_type ,'name:', arg_name)
            params.append(arg_name)
        
        template = build_function_template(func_name,params,return_type)
        if(template in file_chars):
            continue
        
        append_line_count += template.count('\n')
        
        file_chars = file_chars[:define_start+2] + template + file_chars[define_start+2:]
        file_lines = file_lines[:lineno] + template.splitlines(True) + file_lines[lineno:]
        count += 1
        
    output_file = open(file_path, "w")
    output_file.write(file_chars)
    output_file.close()
                 
def build_struct_template(info):
    template = '\n/**\n * @struct '+ info['name'] +'\n'
    template += ' * @brief '+ info['name'] +'\n * \n'

    
    for member in info['members']:
        template += ' * @var ' + info['name'] +'::'+ member + '\n * Please Place Description here. \n'
    
    template += ' */\n'
    return template
    
    
def struct_add_comment(file_path):
    if(file_path.startswith('./os')):
            category = 'OS'
    else:
        category = ''
        pass
    os.system('ctags -x  --sort=no --c-types=+p ' + file_path + ' > ./tmp')
    dump_info  = open('./tmp', 'r') 
    # dump_info  = dump_info.read()
    dump_info_lines = dump_info.readlines()
    src_file = open(file_path, 'r') 
    file_lines = src_file.readlines()
    src_file.seek(0)
    file_chars = src_file.read()
    src_file.close()
    
    struct_regex = r"(\w*)[ ]*struct[ ]*([0-9]+) (.*\.h) .*"
    member_regex = r"(\w*)[ ]*member[ ]*([0-9]+) (.*\.h) .*"
    struct_infos = list()
        
    for i in range(len(dump_info_lines)):
        match = re.match(struct_regex,dump_info_lines[i])
        if(match):
            struct_info = dict()
            struct_info['name'] =  match.group(1)       
            struct_info['members'] = list()
            struct_info['lineno'] = int(match.group(2))
            member_start_line = 0
            member_end_line = 0
            
            brace_count = 0
            for j in range(len(file_lines[struct_info['lineno']-1:])):
                line = file_lines[struct_info['lineno']-1+j]
                if '{' in line:
                    member_start_line   = struct_info['lineno']-1+j
                    brace_count += 1
                if '}' in line:
                    brace_count -= 1
                    if( brace_count == 0 ):
                        member_end_line     = struct_info['lineno']-1+j + 1
                        break
            
            member_line = i + 1
            for j in range(member_line,len(dump_info_lines)):
                member = re.match(member_regex,dump_info_lines[j])
                if(member != None):
                    
                    member_lineno = int(member.group(2))
                    # print(dump_info_lines[j])
                    if(member_lineno <= member_end_line):
                        struct_info['members'].append(member.group(1))
                    else:
                        break
            struct_infos.append(struct_info)
            
            
            
    append_line_count = 0        
    for info in struct_infos:
        lineno = info['lineno'] + append_line_count
        struct_name = info['name']
        template = build_struct_template(info)
        if(template in file_chars):
            continue
        index = get_index_of_line_start(file_lines[:lineno])
        append_line_count += template.count('\n')
        file_chars = file_chars[:index] + template + file_chars[index:]
        file_lines = file_lines[:lineno] + template.splitlines(True) + file_lines[lineno:]
    output_file = open(file_path, "w")
    output_file.write(file_chars)
    output_file.close()


def build_enum_template(info):
    
    template = '\n/**\n * @enum '+ info['name'] +'\n * \n'
    template += ' * @brief '+ info['name'] +'\n * \n'

    for member in info['members']:
        template += ' * @var ' + info['name'] +'::'+ member + '\n * Please Place Description here. \n'
    
    template += ' */\n'
    return template
    
    
def enum_add_comment(file_path):
    if(file_path.startswith('./os')):
            category = 'OS'
    else:
        category = ''
        pass
    os.system('ctags -x  --sort=no --c-types=+p ' + file_path + ' > ./tmp')
    dump_info  = open('./tmp', 'r') 
    # dump_info  = dump_info.read()
    dump_info_lines = dump_info.readlines()
    src_file = open(file_path, 'r') 
    file_lines = src_file.readlines()
    src_file.seek(0)
    file_chars = src_file.read()
    src_file.close()
    
    enum_regex = r"(\w*)[ ]*enum[ ]*([0-9]+) (.*\.h) .*"
    member_regex = r"(\w*)[ ]*enumerator[ ]*([0-9]+) (.*\.h) .*"
    enum_infos = list()

    for i in range(len(dump_info_lines)):
        match = re.match(enum_regex,dump_info_lines[i])
            
        if(match):
            print(match.group(1))        
        
            enum_info = dict()
            enum_info['name'] =  match.group(1)
            enum_info['members'] = list()
            enum_info['lineno'] = int(match.group(2))
            
            member_start_line = 0
            member_end_line = 0
            
            for j in range(len(file_lines[enum_info['lineno']-1:])):
                line = file_lines[enum_info['lineno']-1+j]
                if '{' in line:
                    member_start_line   = enum_info['lineno']-1+j
                
                if '}' in line:
                    member_end_line     = enum_info['lineno']-1+j + 1
                    break
            member_line = i + 1 
                    
            for j in range(member_line,len(dump_info_lines)):
                member = re.match(member_regex,dump_info_lines[j])
                if(member != None):
                    
                    member_lineno = int(member.group(2))
                    # print(member_lineno,member_end_line)
                    # print(dump_info_lines[j])
                    if(member_lineno <= member_end_line):
                        enum_info['members'].append(member.group(1))
                    else:
                        break
            enum_infos.append(enum_info)
            
    
    append_line_count = 0
    documented = 0      
    for info in enum_infos:
        lineno = info['lineno'] + append_line_count
        struct_name = info['name']
        index = get_index_of_line_start(file_lines[:lineno])
        
        template = build_enum_template(info)
        if(template in file_chars):
            continue
        append_line_count += template.count('\n')
        index = get_index_of_line_start(file_lines[:lineno])
        file_chars = file_chars[:index] + template + file_chars[index:]
        file_lines = file_lines[:lineno] + template.splitlines(True) + file_lines[lineno:]
    
    output_file = open(file_path, "w")
    output_file.write(file_chars)
    output_file.close()
    
      
def build_macro_template(info):
    
    template = '\n/**\n * @def '+ info['name'] +'\n * \n'
    
    template += ' */\n'
    return template


def macro_add_comment(file_path):
    if(file_path.startswith('./os')):
            category = 'OS'
    else:
        category = ''
        pass
    os.system('ctags -x  --sort=no --c-types=+p ' + file_path + ' > ./tmp')
    dump_info  = open('./tmp', 'r') 
    # dump_info  = dump_info.read()
    dump_info_lines = dump_info.readlines()
    src_file = open(file_path, 'r') 
    file_lines = src_file.readlines()
    src_file.seek(0)
    file_chars = src_file.read()
    src_file.close()
    
    macro_regex = r"(\w*)[ ]*macro[ ]*([0-9]+) (.*\.h) .*"
    macro_infos = list()

    for i in range(len(dump_info_lines)):
        match = re.match(macro_regex,dump_info_lines[i])
        
        if(match):
            macro_info = dict()
            macro_info['name'] =  match.group(1)      
            macro_info['lineno'] = int(match.group(2))
            macro_infos.append(macro_info)
    append_line_count = 0
    documented = 0      
    for info in macro_infos:
        lineno = info['lineno'] + append_line_count
        struct_name = info['name']
        index = get_index_of_line_start(file_lines[:lineno])
        
        template = build_macro_template(info)
        if(template in file_chars):
            continue
        append_line_count += template.count('\n')
        index = get_index_of_line_start(file_lines[:lineno])
        file_chars = file_chars[:index] + template + file_chars[index:]
        file_lines = file_lines[:lineno] + template.splitlines(True) + file_lines[lineno:]
    
    output_file = open(file_path, "w")
    output_file.write(file_chars)
    output_file.close()     
def add_file_comment(file_path):
    
    src_file = open(file_path, 'r') 
    file_chars = src_file.read()
    src_file.close()
    

    if(not file_chars.startswith('/** @file */')):
        file_chars = '/** @file */\n' + file_chars
    
    
    output_file = open(file_path, "w")
    output_file.write(file_chars)
    output_file.close()     
    
def remove_whitespace(file_path):
    
    
    src_file = open(file_path, 'r') 
    file_chars = src_file.read()
    src_file.close()
    
    for index in range(len(file_chars)):
        
        if(file_chars[index:index+len('Please Place Description here. ')] == 'Please Place Description here. '):
            file_chars = file_chars[:index] + 'Please Place Description here.' + file_chars[index+len('Please Place Description here. '):]
        
        if(file_chars[index:index+len(' \n')] == ' \n'):
            file_chars = file_chars[:index] + '\n.' + file_chars[index+len(' \n'):]

    output_file = open(file_path, "w")
    output_file.write(file_chars)
    output_file.close()     
if __name__ == '__main__':
    
    # file_path = 'mac_hw_info.h'
    
    
    # # function_add_comment(file_path)
    # # struct_add_comment(file_path)
    # enum_add_comment(file_path)
                
    
    # exit()
    for dirpath, dnames, fnames in os.walk("./"):
        for f in fnames:
            if f.endswith(".h"):
                file_path = os.path.join(dirpath, f)
                print(file_path)
                add_file_comment(file_path)
                function_add_comment(file_path)
                struct_add_comment(file_path)
                enum_add_comment(file_path)
                remove_whitespace(file_path)
                # macro_add_comment(file_path)