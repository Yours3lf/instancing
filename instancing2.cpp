#include "framework.h"

using namespace prototyper;

int main( int argc, char** argv )
{
  map<string, string> args;

  for( int c = 1; c < argc; ++c )
  {
    args[argv[c]] = c + 1 < argc ? argv[c + 1] : "";
    ++c;
  }

  cout << "Arguments: " << endl;
  for_each( args.begin(), args.end(), []( pair<string, string> p )
  {
    cout << p.first << " " << p.second << endl;
  } );

  uvec2 screen( 0 );
  bool fullscreen = false;
  bool silent = false;
  string title = "Instancing2";

  /*
   * Process program arguments
   */

  stringstream ss;
  ss.str( args["--screenx"] );
  ss >> screen.x;
  ss.clear();
  ss.str( args["--screeny"] );
  ss >> screen.y;
  ss.clear();

  if( screen.x == 0 )
  {
    screen.x = 1280;
  }

  if( screen.y == 0 )
  {
    screen.y = 720;
  }

  try
  {
    args.at( "--fullscreen" );
    fullscreen = true;
  }
  catch( ... ) {}

  try
  {
    args.at( "--help" );
    cout << title << ", written by Martin Thomas." << endl <<
         "Usage: --silent      //don't display FPS info in the terminal" << endl <<
         "       --screenx num //set screen width (default:1280)" << endl <<
         "       --screeny num //set screen height (default:720)" << endl <<
         "       --fullscreen  //set fullscreen, windowed by default" << endl <<
         "       --help        //display this information" << endl;
    return 0;
  }
  catch( ... ) {}

  try
  {
    args.at( "--silent" );
    silent = true;
  }
  catch( ... ) {}

  /*
   * Initialize the OpenGL context
   */

  framework frm;
  frm.init( screen, title, fullscreen );

  //set opengl settings
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
  glFrontFace( GL_CCW );
  glEnable( GL_CULL_FACE );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glClearDepth( 1.0f );

  frm.get_opengl_error();

  /*
   * Set up mymath
   */

  camera<float> cam;
  frame<float> the_frame;

  float cam_fov = 45.0f;
  float cam_near = 1.0f;
  float cam_far = 20.0f;

  the_frame.set_perspective( radians( cam_fov ), ( float )screen.x / ( float )screen.y, cam_near, cam_far );

  glViewport( 0, 0, screen.x, screen.y );

  /*
   * Set up the scene
   */

  vec3 box_pos( 0, 0, -8 );

  camera<float> light_cam;
  vec3 light_pos( 0, 5, -5 );
  vec3 light_rotation( 20, 0, 0 );

  light_cam.move_forward( light_pos.z );
  light_cam.move_up( light_pos.y );
  light_cam.rotate_x( radians( light_rotation.x ) );
  light_cam.rotate_y( radians( light_rotation.y ) );
  light_cam.rotate_z( radians( light_rotation.z ) );

  float light_exponent = 100;
  float light_radius = 20;
  float light_fov = 45.0f;

  float cam_rotation_amount = 5;
  float move_amount = 1;

  GLuint quad = frm.create_quad( the_frame.far_ll.xyz, the_frame.far_lr.xyz, the_frame.far_ul.xyz, the_frame.far_ur.xyz );
  GLuint box = frm.create_box(); //Vertex Array Object (VAO) of the box

  /*
   * Set up instancing
   */

  int size = 100;
  cout << "Rendering: " << size* size << " cubes" << endl;

  glBindVertexArray( box );

  vector<mat4> positions;
  positions.resize( size * size );

  GLuint position_vbo;
  glGenBuffers( 1, &position_vbo ); //gen vbo
  glBindBuffer( GL_ARRAY_BUFFER, position_vbo ); //bind vbo

  GLuint location = 3;
  GLint components = 4;
  GLenum type = GL_FLOAT;
  GLboolean normalized = GL_FALSE;
  GLsizei datasize = sizeof( mat4 );
  char* pointer = 0;
  GLuint divisor = 1;

  /**
  Matrix:
  float mat[16] =
  {
    1, 0, 0, 0, //first column:  location at 0 + 0 * sizeof( vec4 ) bytes into the matrix
    0, 1, 0, 0, //second column: location at 0 + 1 * sizeof( vec4 ) bytes into the matrix
    0, 0, 1, 0, //third column:  location at 0 + 2 * sizeof( vec4 ) bytes into the matrix
    0, 0, 0, 1  //fourth column  location at 0 + 3 * sizeof( vec4 ) bytes into the matrix
  };
  /**/

  //you need to do everything for each vertex attribute location
  for( int c = 0; c < 4; ++c )
  {
    glEnableVertexAttribArray( location + c ); //location of each column
    glVertexAttribPointer( location + c, components, type, normalized, datasize, pointer + c * sizeof( vec4 ) ); //tell other data
    //glVertexAttribPointer( location + c, components, type, normalized, datasize, (char*)c + c * sizeof( vec4 ) ); //wrong
    glVertexAttribDivisor( location + c, divisor ); //is it instanced?
  }

  bool do_instancing = true;

  frm.get_opengl_error();

  /*
   * Set up the shaders
   */

  GLuint gbuffer_shader = 0;
  frm.load_shader( gbuffer_shader, GL_VERTEX_SHADER, "../shaders/instancing2/gbuffer.vs" );
  frm.load_shader( gbuffer_shader, GL_FRAGMENT_SHADER, "../shaders/instancing2/gbuffer.ps" );

  GLuint gbuffer_instanced_shader = 0;
  frm.load_shader( gbuffer_instanced_shader, GL_VERTEX_SHADER, "../shaders/instancing2/gbuffer_instanced_mat.vs" );
  frm.load_shader( gbuffer_instanced_shader, GL_FRAGMENT_SHADER, "../shaders/instancing2/gbuffer.ps" );

  GLuint light_shader = 0;
  frm.load_shader( light_shader, GL_VERTEX_SHADER, "../shaders/instancing2/light.vs" );
  frm.load_shader( light_shader, GL_FRAGMENT_SHADER, "../shaders/instancing2/light.ps" );

  GLint gbuffer_mvp_loc = glGetUniformLocation( gbuffer_shader, "mvp" );
  GLint gbuffer_normal_mat_loc = glGetUniformLocation( gbuffer_shader, "normal_mat" );
  GLint gbuffer_pos_loc = glGetUniformLocation( gbuffer_shader, "pos" );

  GLint gbuffer_instanced_normal_mat_loc = glGetUniformLocation( gbuffer_instanced_shader, "normal_mat" );

  GLint light_mvp_loc = glGetUniformLocation( light_shader, "mvp" );
  GLint light_mv_loc = glGetUniformLocation( light_shader, "mv" );
  GLint light_pos_loc = glGetUniformLocation( light_shader, "light_pos" );
  GLint light_spot_dir_loc = glGetUniformLocation( light_shader, "spot_dir" );
  GLint light_radius_loc = glGetUniformLocation( light_shader, "radius" );
  GLint light_spot_cos_cutoff_loc = glGetUniformLocation( light_shader, "spot_cos_cutoff" );
  GLint light_spot_exponent_loc = glGetUniformLocation( light_shader, "spot_exponent" );
  GLint light_color_loc = glGetUniformLocation( light_shader, "light_color" );

  frm.get_opengl_error();

  /*
   * Set up textures
   */

  glBindFramebuffer( GL_FRAMEBUFFER, 0 );

  GLuint depth_texture = 0;
  glGenTextures( 1, &depth_texture );

  glBindTexture( GL_TEXTURE_2D, depth_texture );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, screen.x, screen.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );

  GLuint normal_texture = 0;
  glGenTextures( 1, &normal_texture );

  glBindTexture( GL_TEXTURE_2D, normal_texture );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, screen.x, screen.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );

  /*
   * Set up fbos
   */

  GLuint gbuffer_fbo = 0;
  glGenFramebuffers( 1, &gbuffer_fbo );
  glBindFramebuffer( GL_FRAMEBUFFER, gbuffer_fbo );
  glDrawBuffer( GL_COLOR_ATTACHMENT0 );

  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normal_texture, 0 );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0 );

  frm.check_fbo_status();

  glBindFramebuffer( GL_FRAMEBUFFER, 0 );

  frm.get_opengl_error();

  /*
   * Handle events
   */

  auto event_handler = [&]( const sf::Event & ev )
  {
    switch( ev.type )
    {
      case sf::Event::KeyPressed:
        {
          if( ev.key.code == sf::Keyboard::A )
          {
            cam.rotate_y( radians( cam_rotation_amount ) );
          }

          if( ev.key.code == sf::Keyboard::D )
          {
            cam.rotate_y( radians( -cam_rotation_amount ) );
          }

          if( ev.key.code == sf::Keyboard::W )
          {
            cam.move_forward( move_amount );
          }

          if( ev.key.code == sf::Keyboard::S )
          {
            cam.move_forward( -move_amount );
          }

          if( ev.key.code == sf::Keyboard::Space )
          {
            do_instancing = !do_instancing;
          }
        }
      default:
        break;
    }
  };

  /*
   * Render
   */

  sf::Clock timer;
  timer.restart();

  frm.display( [&]
  {
    frm.handle_events( event_handler );

    mat4 view = cam.get_matrix();
    mat4 model = mat4::identity;
    mat4 projection = the_frame.projection_matrix;
    mat4 normal_mat = view;
    mat4 mvp = projection * view * model;

    //render the scene into the g-buffer
    glEnable( GL_DEPTH_TEST );
    glBindFramebuffer( GL_FRAMEBUFFER, gbuffer_fbo );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    GLuint current_shader = do_instancing ? gbuffer_instanced_shader : gbuffer_shader;

    glUseProgram( current_shader );

    //draw box
    glUniformMatrix4fv( do_instancing ? gbuffer_instanced_normal_mat_loc : gbuffer_normal_mat_loc, 1, false, &normal_mat[0][0] );

    if( !do_instancing )
      glUniformMatrix4fv( gbuffer_mvp_loc, 1, false, &mvp[0][0] );

    if( !do_instancing )
    {
      /**/
      //regular rendering
      glBindVertexArray( box );

      for( int c = 0; c < size; ++c )
      {
        for( int d = 0; d < size; ++d )
        {
          glUniform4f( gbuffer_pos_loc, c * 3 - size, -2 + 0.5 * sin( radians( ( c + d + 1 )* timer.getElapsedTime().asSeconds() ) ), -d * 3, 0 );
          //glUniform4f( gbuffer_pos_loc, c * 3 - size, -2, -d * 3, 0 );
          glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0 );
        }
      }

      /**/
    }
    else
    {
      /**/
      //instanced rendering
      glBindVertexArray( box );

      for( int c = 0; c < size; ++c )
      {
        for( int d = 0; d < size; ++d )
        {
          //mvm.translate_matrix(vec3(c * 3 - size, -2, -d * 3));
          mat4 trans = create_translation( vec3( c * 3 - size, -2 + 0.5 * sin( radians( ( c + d + 1 )* timer.getElapsedTime().asSeconds() ) ), -d * 3 ) );
          positions[c * size + d] = mvp * trans;
        }
      }

      glBindBuffer( GL_ARRAY_BUFFER, position_vbo ); //bind vbo
      glBufferData( GL_ARRAY_BUFFER, sizeof( mat4 ) * positions.size(), &positions[0][0], GL_DYNAMIC_DRAW );

      glDrawElementsInstanced( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, positions.size() );
      /**/
    }


    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    //render the lights
    glDisable( GL_DEPTH_TEST );
    glUseProgram( light_shader );

    //pass the light properties in view space
    //vec3 light_pos = ( ppl.get_model_view_matrix() * vec4( light_cam.pos, 1 ) ).xyz;
    //vec3 spot_dir = ( ppl.get_model_view_matrix() * vec4( light_cam.view_dir, 0 ) ).xyz;
    vec3 light_pos = ( view * vec4( cam.pos, 1 ) ).xyz;
    vec3 spot_dir = ( view * vec4( normalize( cam.view_dir - vec3( 0, 0.25, 0 ) ), 0 ) ).xyz;

    glUniform3fv( light_pos_loc, 1, &light_pos[0] );
    glUniform3fv( light_spot_dir_loc, 1, &spot_dir[0] );
    glUniform1f( light_radius_loc, light_radius );
    glUniform1f( light_spot_cos_cutoff_loc, std::cos( radians( light_fov ) ) );
    glUniform1f( light_spot_exponent_loc, light_exponent );
    vec3 red( 1, 0.25, 0.25 );
    vec3 green( 0.25, 1, 0.25 );
    glUniform3fv( light_color_loc, 1, do_instancing ? &green[0] : &red[0] );

    glBindTexture( GL_TEXTURE_2D, depth_texture );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, normal_texture );
    glActiveTexture( GL_TEXTURE0 );

    glUniformMatrix4fv( light_mvp_loc, 1, false, &projection[0][0] );
    mat4 m = mat4::identity;
    glUniformMatrix4fv( light_mv_loc, 1, false, &m[0][0] );

    glBindVertexArray( quad );
    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

    frm.get_opengl_error();

  }, silent );

  return 0;
}
