namespace HugeFileTransfer
{
    partial class Form1
    {
        /// <summary>
        /// Variable del diseñador requerida.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Limpiar los recursos que se estén utilizando.
        /// </summary>
        /// <param name="disposing">true si los recursos administrados se deben desechar; false en caso contrario.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Código generado por el Diseñador de Windows Forms

        /// <summary>
        /// Método necesario para admitir el Diseñador. No se puede modificar
        /// el contenido del método con el editor de código.
        /// </summary>
        private void InitializeComponent()
        {
            this.bgTransfer = new System.ComponentModel.BackgroundWorker();
            this.tabOperationMode = new System.Windows.Forms.TabControl();
            this.tabUpload = new System.Windows.Forms.TabPage();
            this.grpResult = new System.Windows.Forms.GroupBox();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.lblResProgress = new System.Windows.Forms.Label();
            this.lblHeaderProgress = new System.Windows.Forms.Label();
            this.lblResCurrentSize = new System.Windows.Forms.Label();
            this.lblResRemaining = new System.Windows.Forms.Label();
            this.lblResKBps = new System.Windows.Forms.Label();
            this.lblHeaderCurrentSize = new System.Windows.Forms.Label();
            this.lblHeaderRemaining = new System.Windows.Forms.Label();
            this.lblHeaderKBps = new System.Windows.Forms.Label();
            this.progBarUpload = new System.Windows.Forms.ProgressBar();
            this.btnExaminar = new System.Windows.Forms.Button();
            this.txtServerFileName = new System.Windows.Forms.TextBox();
            this.txtClientFileName = new System.Windows.Forms.TextBox();
            this.lblServerFileName = new System.Windows.Forms.Label();
            this.lblClientFileName = new System.Windows.Forms.Label();
            this.btnUpload = new System.Windows.Forms.Button();
            this.tabDownload = new System.Windows.Forms.TabPage();
            this.barraEstado = new System.Windows.Forms.StatusStrip();
            this.dlgOpenFile = new System.Windows.Forms.OpenFileDialog();
            this.grpServerFiles = new System.Windows.Forms.GroupBox();
            this.lstServerFiles = new System.Windows.Forms.TreeView();
            this.bgMonitor = new System.ComponentModel.BackgroundWorker();
            this.lstResult = new System.Windows.Forms.ListBox();
            this.tabOperationMode.SuspendLayout();
            this.tabUpload.SuspendLayout();
            this.grpResult.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.grpServerFiles.SuspendLayout();
            this.SuspendLayout();
            // 
            // bgTransfer
            // 
            this.bgTransfer.WorkerReportsProgress = true;
            this.bgTransfer.DoWork += new System.ComponentModel.DoWorkEventHandler(this.bgTransfer_DoWork);
            this.bgTransfer.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.bgTransfer_ProgressChanged);
            // 
            // tabOperationMode
            // 
            this.tabOperationMode.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabOperationMode.Controls.Add(this.tabUpload);
            this.tabOperationMode.Controls.Add(this.tabDownload);
            this.tabOperationMode.HotTrack = true;
            this.tabOperationMode.Location = new System.Drawing.Point(12, 202);
            this.tabOperationMode.Name = "tabOperationMode";
            this.tabOperationMode.SelectedIndex = 0;
            this.tabOperationMode.Size = new System.Drawing.Size(592, 211);
            this.tabOperationMode.TabIndex = 3;
            // 
            // tabUpload
            // 
            this.tabUpload.Controls.Add(this.grpResult);
            this.tabUpload.Controls.Add(this.btnExaminar);
            this.tabUpload.Controls.Add(this.txtServerFileName);
            this.tabUpload.Controls.Add(this.txtClientFileName);
            this.tabUpload.Controls.Add(this.lblServerFileName);
            this.tabUpload.Controls.Add(this.lblClientFileName);
            this.tabUpload.Controls.Add(this.btnUpload);
            this.tabUpload.Location = new System.Drawing.Point(4, 22);
            this.tabUpload.Name = "tabUpload";
            this.tabUpload.Padding = new System.Windows.Forms.Padding(3);
            this.tabUpload.Size = new System.Drawing.Size(584, 185);
            this.tabUpload.TabIndex = 0;
            this.tabUpload.Text = "Subir archivo";
            this.tabUpload.UseVisualStyleBackColor = true;
            // 
            // grpResult
            // 
            this.grpResult.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.grpResult.Controls.Add(this.lstResult);
            this.grpResult.Controls.Add(this.tableLayoutPanel1);
            this.grpResult.Location = new System.Drawing.Point(9, 58);
            this.grpResult.Name = "grpResult";
            this.grpResult.Size = new System.Drawing.Size(569, 121);
            this.grpResult.TabIndex = 14;
            this.grpResult.TabStop = false;
            this.grpResult.Text = "Progreso y resultado de la operación";
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 5;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 125F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 100F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 80F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 60F));
            this.tableLayoutPanel1.Controls.Add(this.lblResProgress, 4, 1);
            this.tableLayoutPanel1.Controls.Add(this.lblHeaderProgress, 3, 0);
            this.tableLayoutPanel1.Controls.Add(this.lblResCurrentSize, 2, 1);
            this.tableLayoutPanel1.Controls.Add(this.lblResRemaining, 1, 1);
            this.tableLayoutPanel1.Controls.Add(this.lblResKBps, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.lblHeaderCurrentSize, 2, 0);
            this.tableLayoutPanel1.Controls.Add(this.lblHeaderRemaining, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.lblHeaderKBps, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.progBarUpload, 3, 1);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(6, 19);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(557, 40);
            this.tableLayoutPanel1.TabIndex = 1;
            // 
            // lblResProgress
            // 
            this.lblResProgress.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblResProgress.Location = new System.Drawing.Point(500, 20);
            this.lblResProgress.Name = "lblResProgress";
            this.lblResProgress.Size = new System.Drawing.Size(54, 20);
            this.lblResProgress.TabIndex = 9;
            this.lblResProgress.Text = "0.00 %";
            this.lblResProgress.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // lblHeaderProgress
            // 
            this.tableLayoutPanel1.SetColumnSpan(this.lblHeaderProgress, 2);
            this.lblHeaderProgress.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblHeaderProgress.Location = new System.Drawing.Point(308, 0);
            this.lblHeaderProgress.Name = "lblHeaderProgress";
            this.lblHeaderProgress.Size = new System.Drawing.Size(246, 20);
            this.lblHeaderProgress.TabIndex = 7;
            this.lblHeaderProgress.Text = "Progreso de la subida";
            this.lblHeaderProgress.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblResCurrentSize
            // 
            this.lblResCurrentSize.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblResCurrentSize.Location = new System.Drawing.Point(228, 20);
            this.lblResCurrentSize.Name = "lblResCurrentSize";
            this.lblResCurrentSize.Size = new System.Drawing.Size(74, 20);
            this.lblResCurrentSize.TabIndex = 6;
            this.lblResCurrentSize.Text = "0 KB";
            this.lblResCurrentSize.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblResRemaining
            // 
            this.lblResRemaining.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblResRemaining.Location = new System.Drawing.Point(128, 20);
            this.lblResRemaining.Name = "lblResRemaining";
            this.lblResRemaining.Size = new System.Drawing.Size(94, 20);
            this.lblResRemaining.TabIndex = 5;
            this.lblResRemaining.Text = "N/A";
            this.lblResRemaining.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblResKBps
            // 
            this.lblResKBps.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblResKBps.Location = new System.Drawing.Point(3, 20);
            this.lblResKBps.Name = "lblResKBps";
            this.lblResKBps.Size = new System.Drawing.Size(119, 20);
            this.lblResKBps.TabIndex = 4;
            this.lblResKBps.Text = "0 KB/s";
            this.lblResKBps.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblHeaderCurrentSize
            // 
            this.lblHeaderCurrentSize.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblHeaderCurrentSize.Location = new System.Drawing.Point(228, 0);
            this.lblHeaderCurrentSize.Name = "lblHeaderCurrentSize";
            this.lblHeaderCurrentSize.Size = new System.Drawing.Size(74, 20);
            this.lblHeaderCurrentSize.TabIndex = 2;
            this.lblHeaderCurrentSize.Text = "Total subido";
            this.lblHeaderCurrentSize.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblHeaderRemaining
            // 
            this.lblHeaderRemaining.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblHeaderRemaining.Location = new System.Drawing.Point(128, 0);
            this.lblHeaderRemaining.Name = "lblHeaderRemaining";
            this.lblHeaderRemaining.Size = new System.Drawing.Size(94, 20);
            this.lblHeaderRemaining.TabIndex = 1;
            this.lblHeaderRemaining.Text = "Tiempo restante";
            this.lblHeaderRemaining.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // lblHeaderKBps
            // 
            this.lblHeaderKBps.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lblHeaderKBps.Location = new System.Drawing.Point(3, 0);
            this.lblHeaderKBps.Name = "lblHeaderKBps";
            this.lblHeaderKBps.Size = new System.Drawing.Size(119, 20);
            this.lblHeaderKBps.TabIndex = 0;
            this.lblHeaderKBps.Text = "Velocidad de subida";
            this.lblHeaderKBps.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // progBarUpload
            // 
            this.progBarUpload.Dock = System.Windows.Forms.DockStyle.Fill;
            this.progBarUpload.Location = new System.Drawing.Point(308, 23);
            this.progBarUpload.Name = "progBarUpload";
            this.progBarUpload.Size = new System.Drawing.Size(186, 14);
            this.progBarUpload.TabIndex = 8;
            // 
            // btnExaminar
            // 
            this.btnExaminar.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnExaminar.Location = new System.Drawing.Point(494, 6);
            this.btnExaminar.Name = "btnExaminar";
            this.btnExaminar.Size = new System.Drawing.Size(84, 20);
            this.btnExaminar.TabIndex = 13;
            this.btnExaminar.Text = "Examinar";
            this.btnExaminar.UseVisualStyleBackColor = true;
            this.btnExaminar.Click += new System.EventHandler(this.btnExaminar_Click);
            // 
            // txtServerFileName
            // 
            this.txtServerFileName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtServerFileName.Location = new System.Drawing.Point(180, 32);
            this.txtServerFileName.Name = "txtServerFileName";
            this.txtServerFileName.Size = new System.Drawing.Size(308, 20);
            this.txtServerFileName.TabIndex = 11;
            this.txtServerFileName.Text = "Subidas/prueba.avi";
            // 
            // txtClientFileName
            // 
            this.txtClientFileName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.txtClientFileName.Enabled = false;
            this.txtClientFileName.Location = new System.Drawing.Point(180, 6);
            this.txtClientFileName.Name = "txtClientFileName";
            this.txtClientFileName.Size = new System.Drawing.Size(308, 20);
            this.txtClientFileName.TabIndex = 12;
            this.txtClientFileName.Text = "D:/Videos/Natacion casa de campo.avi";
            // 
            // lblServerFileName
            // 
            this.lblServerFileName.AutoSize = true;
            this.lblServerFileName.Location = new System.Drawing.Point(6, 35);
            this.lblServerFileName.Name = "lblServerFileName";
            this.lblServerFileName.Size = new System.Drawing.Size(168, 13);
            this.lblServerFileName.TabIndex = 5;
            this.lblServerFileName.Text = "Nombre del archivo en el servidor:";
            // 
            // lblClientFileName
            // 
            this.lblClientFileName.AutoSize = true;
            this.lblClientFileName.Location = new System.Drawing.Point(6, 9);
            this.lblClientFileName.Name = "lblClientFileName";
            this.lblClientFileName.Size = new System.Drawing.Size(136, 13);
            this.lblClientFileName.TabIndex = 7;
            this.lblClientFileName.Text = "Nombre del archivo a subir:";
            // 
            // btnUpload
            // 
            this.btnUpload.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnUpload.Location = new System.Drawing.Point(494, 32);
            this.btnUpload.Name = "btnUpload";
            this.btnUpload.Size = new System.Drawing.Size(84, 20);
            this.btnUpload.TabIndex = 3;
            this.btnUpload.Text = "Subir archivo";
            this.btnUpload.UseVisualStyleBackColor = true;
            this.btnUpload.Click += new System.EventHandler(this.btnUpload_Click);
            // 
            // tabDownload
            // 
            this.tabDownload.Location = new System.Drawing.Point(4, 22);
            this.tabDownload.Name = "tabDownload";
            this.tabDownload.Padding = new System.Windows.Forms.Padding(3);
            this.tabDownload.Size = new System.Drawing.Size(584, 185);
            this.tabDownload.TabIndex = 1;
            this.tabDownload.Text = "Descargar archivo";
            this.tabDownload.UseVisualStyleBackColor = true;
            // 
            // barraEstado
            // 
            this.barraEstado.Location = new System.Drawing.Point(0, 416);
            this.barraEstado.Name = "barraEstado";
            this.barraEstado.Size = new System.Drawing.Size(616, 22);
            this.barraEstado.TabIndex = 4;
            // 
            // grpServerFiles
            // 
            this.grpServerFiles.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.grpServerFiles.Controls.Add(this.lstServerFiles);
            this.grpServerFiles.Location = new System.Drawing.Point(12, 12);
            this.grpServerFiles.Name = "grpServerFiles";
            this.grpServerFiles.Size = new System.Drawing.Size(592, 184);
            this.grpServerFiles.TabIndex = 5;
            this.grpServerFiles.TabStop = false;
            this.grpServerFiles.Text = "Archivos en el servidor";
            // 
            // lstServerFiles
            // 
            this.lstServerFiles.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lstServerFiles.Location = new System.Drawing.Point(13, 19);
            this.lstServerFiles.Name = "lstServerFiles";
            this.lstServerFiles.Size = new System.Drawing.Size(569, 159);
            this.lstServerFiles.TabIndex = 6;
            // 
            // bgMonitor
            // 
            this.bgMonitor.WorkerReportsProgress = true;
            this.bgMonitor.DoWork += new System.ComponentModel.DoWorkEventHandler(this.bgMonitor_DoWork);
            this.bgMonitor.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.bgMonitor_ProgressChanged);
            // 
            // lstResult
            // 
            this.lstResult.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lstResult.FormattingEnabled = true;
            this.lstResult.Location = new System.Drawing.Point(6, 64);
            this.lstResult.Name = "lstResult";
            this.lstResult.Size = new System.Drawing.Size(557, 56);
            this.lstResult.TabIndex = 2;
            // 
            // Form1
            // 
            this.AcceptButton = this.btnUpload;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(616, 438);
            this.Controls.Add(this.grpServerFiles);
            this.Controls.Add(this.barraEstado);
            this.Controls.Add(this.tabOperationMode);
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "HugeFileTransfer";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.tabOperationMode.ResumeLayout(false);
            this.tabUpload.ResumeLayout(false);
            this.tabUpload.PerformLayout();
            this.grpResult.ResumeLayout(false);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.grpServerFiles.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.ComponentModel.BackgroundWorker bgTransfer;
        private System.Windows.Forms.TabControl tabOperationMode;
        private System.Windows.Forms.TabPage tabUpload;
        private System.Windows.Forms.GroupBox grpResult;
        private System.Windows.Forms.Button btnExaminar;
        private System.Windows.Forms.TextBox txtServerFileName;
        private System.Windows.Forms.TextBox txtClientFileName;
        private System.Windows.Forms.Label lblServerFileName;
        private System.Windows.Forms.Label lblClientFileName;
        private System.Windows.Forms.Button btnUpload;
        private System.Windows.Forms.TabPage tabDownload;
        private System.Windows.Forms.StatusStrip barraEstado;
        private System.Windows.Forms.OpenFileDialog dlgOpenFile;
        private System.Windows.Forms.GroupBox grpServerFiles;
        private System.Windows.Forms.TreeView lstServerFiles;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Label lblResRemaining;
        private System.Windows.Forms.Label lblResKBps;
        private System.Windows.Forms.Label lblHeaderCurrentSize;
        private System.Windows.Forms.Label lblHeaderRemaining;
        private System.Windows.Forms.Label lblHeaderKBps;
        private System.Windows.Forms.Label lblResProgress;
        private System.Windows.Forms.Label lblHeaderProgress;
        private System.Windows.Forms.Label lblResCurrentSize;
        private System.Windows.Forms.ProgressBar progBarUpload;
        private System.ComponentModel.BackgroundWorker bgMonitor;
        private System.Windows.Forms.ListBox lstResult;
    }
}

