import os

single_header_proto_folder = 'single_header_src'
prototype_prefix           = 'proto_'
license                    = '/*\n' + open( 'LICENSE', 'r' ).read() + '*/\n'
license_notice = """
//---------------------------------------------------------//
//-- LICENSE -- See end of file for license information  --//
//---------------------------------------------------------//
"""[1:]
for f in filter( lambda s: s.startswith(prototype_prefix) and s.endswith('.h'), os.listdir( single_header_proto_folder ) ):
  print( f )
  input_filename  = '/'.join([single_header_proto_folder,f])
  output_filename = f[len(prototype_prefix):]
  input_file  = open( input_filename, 'r' ).read()
  output_file = open( output_filename, 'w+' )

  for line in input_file.split( '\n' ):
    if line.startswith('#include'):
      src_filename = line[line.find('"')+1:line.rfind('"')]
      src_file = open( '/'.join([single_header_proto_folder,src_filename]), 'r' ).read()
      print( output_filename, '<--', src_filename )
      assert( src_file.endswith( license ) )
      assert( src_file.startswith( license_notice ) )
      src_file = src_file[ len(license_notice):-len(license) ]
      output_file.write( src_file )
    else:
      output_file.write( line )
