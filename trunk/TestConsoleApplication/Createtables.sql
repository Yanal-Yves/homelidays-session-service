IF OBJECT_ID (N'[dbo].[Session]', N'U') IS NULL
	CREATE TABLE [dbo].[Session](
		[SessionId] [uniqueidentifier] NOT NULL,
		[LastAccessed] [datetime] NOT NULL,
		[SessionTimeOut] int NOT NULL,
		[Data] [nvarchar](max) NULL,
	 CONSTRAINT [PK_Session] PRIMARY KEY CLUSTERED 
	(
		[SessionId] ASC
	)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
	) ON [PRIMARY]