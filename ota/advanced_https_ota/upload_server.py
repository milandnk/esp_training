import os
from http.server import BaseHTTPRequestHandler, HTTPServer
import cgi

UPLOAD_DIR = "./uploads"  # Directory to save uploaded files
os.makedirs(UPLOAD_DIR, exist_ok=True)

class FileUploadHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        html = """
        <html>
            <body>
                <h1>Upload Firmware</h1>
                <form method="POST" enctype="multipart/form-data" action="/">
                    <input type="file" name="file" />
                    <br><br>
                    <input type="submit" value="Upload" />
                </form>
            </body>
        </html>
        """
        self.wfile.write(html.encode("utf-8"))

    def do_POST(self):
        content_type = self.headers.get("Content-Type")
        if not content_type.startswith("multipart/form-data"):
            self.send_error(400, "Bad Request: Expected multipart form data")
            return

        form = cgi.FieldStorage(fp=self.rfile, headers=self.headers, environ={"REQUEST_METHOD": "POST"})
        file_item = form["file"]
        if file_item.filename:
            file_path = os.path.join(UPLOAD_DIR, file_item.filename)
            with open(file_path, "wb") as f:
                f.write(file_item.file.read())
            self.send_response(200)
            self.end_headers()
            self.wfile.write(f"File uploaded successfully to {file_path}".encode("utf-8"))
        else:
            self.send_error(400, "Bad Request: No file uploaded")

def run(server_class=HTTPServer, handler_class=FileUploadHandler, port=8000):
    server_address = ("", port)
    httpd = server_class(server_address, handler_class)
    print(f"Starting server on port {port}")
    httpd.serve_forever()

if __name__ == "__main__":
    run()
# import os
# from http.server import BaseHTTPRequestHandler, HTTPServer
# import cgi

# UPLOAD_DIR = "./uploads"  # Directory to save uploaded files
# os.makedirs(UPLOAD_DIR, exist_ok=True)

# class FileUploadHandler(BaseHTTPRequestHandler):
#     def do_GET(self):
#         if self.path == "/":  # Serve the upload form
#             self.send_response(200)
#             self.send_header("Content-type", "text/html")
#             self.end_headers()
#             html = """
#             <html>
#                 <body>
#                     <h1>Upload Firmware</h1>
#                     <form method="POST" enctype="multipart/form-data" action="/">
#                         <input type="file" name="file" />
#                         <br><br>
#                         <input type="submit" value="Upload" />
#                     </form>
#                 </body>
#             </html>
#             """
#             self.wfile.write(html.encode("utf-8"))
#         else:  # Serve uploaded firmware for OTA
#             file_path = os.path.join(UPLOAD_DIR, os.path.basename(self.path))
#             if os.path.exists(file_path):
#                 self.send_response(200)
#                 self.send_header("Content-Type", "application/octet-stream")
#                 self.end_headers()
#                 with open(file_path, "rb") as f:
#                     self.wfile.write(f.read())
#             else:
#                 self.send_error(404, "File Not Found")

#     def do_POST(self):
#         content_type = self.headers.get("Content-Type")
#         if not content_type.startswith("multipart/form-data"):
#             self.send_error(400, "Bad Request: Expected multipart form data")
#             return

#         form = cgi.FieldStorage(fp=self.rfile, headers=self.headers, environ={"REQUEST_METHOD": "POST"})
#         file_item = form["file"]
#         if file_item.filename:
#             file_path = os.path.join(UPLOAD_DIR, file_item.filename)
#             with open(file_path, "wb") as f:
#                 f.write(file_item.file.read())
#             self.send_response(200)
#             self.end_headers()
#             self.wfile.write(f"File uploaded successfully to {file_path}".encode("utf-8"))
#         else:
#             self.send_error(400, "Bad Request: No file uploaded")

# def run(server_class=HTTPServer, handler_class=FileUploadHandler, port=8000):
#     server_address = ("", port)
#     httpd = server_class(server_address, handler_class)
#     print(f"Starting server on port {port}")
#     httpd.serve_forever()

# if __name__ == "__main__":
#     run()
